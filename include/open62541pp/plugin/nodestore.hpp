#pragma once

#include <cassert>
#include <cstddef>  // size_t
#include <optional>

#include "open62541pp/async_operation.hpp"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/detail/open62541/server.h"  // nodestore plugins defined in server.h before v1.2
#include "open62541pp/detail/result_utils.hpp"  // tryInvoke
#include "open62541pp/detail/server_utils.hpp"  // getSession
#include "open62541pp/exception.hpp"  // BadStatus
#include "open62541pp/plugin/pluginadapter.hpp"
#include "open62541pp/session.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/wrapper.hpp"  // asWrapper

namespace opcua::detail {

// The value source API was renamed in v1.5, the old names are deprecated aliases there.
#if UAPP_OPEN62541_VER_GE(1, 5)
using NativeCallbackValueSource = UA_CallbackValueSource;
using NativeValueSourceNotifications = UA_ValueSourceNotifications;
#else
using NativeCallbackValueSource = UA_DataSource;
using NativeValueSourceNotifications = UA_ValueCallback;
#endif

}  // namespace opcua::detail

namespace opcua {

/**
 * Value callback base class for variable nodes.
 *
 * Value callbacks allow to synchronize a variable value with an external representation.
 * The attached callbacks are executed before every read and after every write operation.
 * @see https://www.open62541.org/doc/1.3/tutorial_server_datasource.html
 *
 * @note Wraps the native `UA_ValueSourceNotifications` (`UA_ValueCallback` before open62541 v1.5).
 */
class ValueCallbackBase : public PluginAdapter<detail::NativeValueSourceNotifications> {
public:
    /**
     * Called before the value attribute is read.
     *
     * It is possible to write into the value attribute during onBeforeRead (using e.g.
     * services::writeValue or Node::writeValue). The node is re-opened afterwards so that changes
     * are considered in the following read operation.
     *
     * @param session Current session
     * @param id The identifier of the node being read from
     * @param range Optional numeric range the client wants to read from
     * @param value Current value before the read operation
     */
    virtual void onRead(
        Session& session, const NodeId& id, const NumericRange* range, const DataValue& value
    ) = 0;

    /**
     * Called after writing the value attribute.
     *
     * The node is re-opened after writing so that the new value is visible in the callback.
     *
     * @param session Current session
     * @param id The identifier of the node being written to
     * @param range Optional numeric range the client wants to write to
     * @param value New value after the write operation
     */
    virtual void onWrite(
        Session& session, const NodeId& id, const NumericRange* range, const DataValue& value
    ) = 0;

    detail::NativeValueSourceNotifications create(bool ownsAdapter) override;
};

/* ----------------------------------------- DataSource ----------------------------------------- */

class DataSourceAdapter : public PluginAdapter<detail::NativeCallbackValueSource> {};

}  // namespace opcua

namespace opcua::detail {

/// Get the data source registered for a node from its NodeContext.
/// Defined in nodestore.cpp to avoid a cyclic include of server_context.hpp.
DataSourceAdapter* getDataSource(void* nodeContext) noexcept;

template <typename Self>
UA_StatusCode dataSourceReadNativeSync(
    UA_Server* server,
    const UA_NodeId* sessionId,
    void* sessionContext,
    const UA_NodeId* nodeId,
    void* nodeContext,
    UA_Boolean includeSourceTimeStamp,
    const UA_NumericRange* range,
    UA_DataValue* value
) noexcept {
    assert(nodeContext != nullptr && nodeId != nullptr && value != nullptr);
    auto* adapter = getDataSource(nodeContext);
    auto session = getSession(server, sessionId, sessionContext);
    if (adapter == nullptr || !session) {
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    auto& self = static_cast<Self&>(*adapter);
    return tryInvoke([&] {
               return self.read(
                   session.value(),
                   asWrapper<NodeId>(*nodeId),
                   asWrapper<NumericRange>(range),
                   asWrapper<DataValue>(*value),
                   includeSourceTimeStamp
               );
           }
    ).code();
}

template <typename Self>
UA_StatusCode dataSourceWriteNativeSync(
    UA_Server* server,
    const UA_NodeId* sessionId,
    void* sessionContext,
    const UA_NodeId* nodeId,
    void* nodeContext,
    const UA_NumericRange* range,
    const UA_DataValue* value
) noexcept {
    assert(nodeContext != nullptr && nodeId != nullptr && value != nullptr);
    auto* adapter = getDataSource(nodeContext);
    auto session = getSession(server, sessionId, sessionContext);
    if (adapter == nullptr || !session) {
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    auto& self = static_cast<Self&>(*adapter);
    return tryInvoke([&] {
               return self.write(
                   session.value(),
                   asWrapper<NodeId>(*nodeId),
                   asWrapper<NumericRange>(range),
                   asWrapper<DataValue>(*value)
               );
           }
    ).code();
}

}  // namespace opcua::detail

namespace opcua {

/**
 * Data source for variable nodes.
 *
 * The server redirects every read and write request to callback functions.
 * Internally, the data source needs to implement its own memory management.
 *
 * The template arguments select which of the operations can be completed asynchronously (see
 * AsyncReadContext and AsyncWriteContext). DataSourceBase is the purely synchronous variant.
 * @see https://www.open62541.org/doc/1.3/tutorial_server_datasource.html
 *
 * @note Wraps the native `UA_CallbackValueSource` (`UA_DataSource` before open62541 v1.5).
 * @note The asynchronous variants require open62541 v1.5 or later and `UA_MULTITHREADING >= 100`.
 */
template <bool IsAsyncRead, bool IsAsyncWrite>
class DataSource;

/// Synchronous data source, see DataSourceBase.
/// @note Wraps the native `UA_CallbackValueSource` (`UA_DataSource` before open62541 v1.5).
template <>
class DataSource<false, false> : public DataSourceAdapter {
public:
    /**
     * Callback to set the read value, the result status and optionally a source timestamp.
     *
     * @note Zero-copy operations are possible. It is not required to return a copy of the actual
     * content data. You can return a pointer to memory owned by the user. Memory can be reused
     * between read callbacks of a data source, as the result is already encoded on the network
     * buffer between each read operation.
     * To use zero-copy reads, set the value of the Variant (DataValue::setValue) without copying,
     * e.g. with Variant::assign.
     *
     * @param session Current session
     * @param id The identifier of the node being read from
     * @param range If not `nullptr`, then the data source shall return only a selection of the
     *              (non-scalar) data.
     *              Set `UA_STATUSCODE_BADINDEXRANGEINVALID` in `value` if this does not apply.
     * @param value The DataValue that is returned to the reader
     * @param timestamp Set the source timestamp of `value` if `true`
     * @return StatusCode
     */
    virtual StatusCode read(
        Session& session,
        const NodeId& id,
        const NumericRange* range,
        DataValue& value,
        bool timestamp
    ) = 0;

    /**
     * Callback to write the value into a data source.
     *
     * @param session Current session
     * @param id The identifier of the node being written to
     * @param range If not `nullptr`, then only this selection of (non-scalar) data should be
     *              written into the data source.
     * @param value The DataValue that has been written by the writer
     * @return StatusCode
     */
    virtual StatusCode write(
        Session& session, const NodeId& id, const NumericRange* range, const DataValue& value
    ) = 0;

    detail::NativeCallbackValueSource create(bool ownsAdapter) override {
        if (ownsAdapter) {
            throw BadStatus(UA_STATUSCODE_BADINTERNALERROR);
        }
        detail::NativeCallbackValueSource native{};
        native.read = &readNative;
        native.write = &writeNative;
        return native;
    }

private:
    static UA_StatusCode readNative(
        UA_Server* server,
        const UA_NodeId* sessionId,
        void* sessionContext,
        const UA_NodeId* nodeId,
        void* nodeContext,
        UA_Boolean includeSourceTimeStamp,
        const UA_NumericRange* range,
        UA_DataValue* value
    ) noexcept {
        return detail::dataSourceReadNativeSync<DataSource<false, false>>(
            server,
            sessionId,
            sessionContext,
            nodeId,
            nodeContext,
            includeSourceTimeStamp,
            range,
            value
        );
    }

    static UA_StatusCode writeNative(
        UA_Server* server,
        const UA_NodeId* sessionId,
        void* sessionContext,
        const UA_NodeId* nodeId,
        void* nodeContext,
        const UA_NumericRange* range,
        const UA_DataValue* value
    ) noexcept {
        return detail::dataSourceWriteNativeSync<DataSource<false, false>>(
            server, sessionId, sessionContext, nodeId, nodeContext, range, value
        );
    }
};

using DataSourceBase = DataSource<false, false>;

}  // namespace opcua

#if UAPP_HAS_ASYNC_OPERATIONS && UAPP_OPEN62541_VER_GE(1, 5)

namespace opcua::detail {

AsyncOperationRegistry& getAsyncOperations(UA_Server* server) noexcept;

template <typename Self>
UA_StatusCode dataSourceReadNativeAsync(
    UA_Server* server,
    const UA_NodeId* sessionId,
    void* sessionContext,
    const UA_NodeId* nodeId,
    void* nodeContext,
    UA_Boolean includeSourceTimeStamp,
    const UA_NumericRange* range,
    UA_DataValue* value
) noexcept {
    assert(nodeContext != nullptr && nodeId != nullptr && value != nullptr);
    auto* adapter = getDataSource(nodeContext);
    auto session = getSession(server, sessionId, sessionContext);
    if (adapter == nullptr || !session) {
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    auto& self = static_cast<Self&>(*adapter);
    AsyncReadContext ctx{server, getAsyncOperations(server), value};
    return tryInvoke([&] {
               return self.read(
                   session.value(),
                   asWrapper<NodeId>(*nodeId),
                   asWrapper<NumericRange>(range),
                   asWrapper<DataValue>(*value),
                   includeSourceTimeStamp,
                   ctx
               );
           }
    ).code();
}

template <typename Self>
UA_StatusCode dataSourceWriteNativeAsync(
    UA_Server* server,
    const UA_NodeId* sessionId,
    void* sessionContext,
    const UA_NodeId* nodeId,
    void* nodeContext,
    const UA_NumericRange* range,
    const UA_DataValue* value
) noexcept {
    assert(nodeContext != nullptr && nodeId != nullptr && value != nullptr);
    auto* adapter = getDataSource(nodeContext);
    auto session = getSession(server, sessionId, sessionContext);
    if (adapter == nullptr || !session) {
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    auto& self = static_cast<Self&>(*adapter);
    AsyncWriteContext ctx{server, getAsyncOperations(server), value};
    return tryInvoke([&] {
               return self.write(
                   session.value(),
                   asWrapper<NodeId>(*nodeId),
                   asWrapper<NumericRange>(range),
                   asWrapper<DataValue>(*value),
                   ctx
               );
           }
    ).code();
}

}  // namespace opcua::detail

namespace opcua {

/// Data source with asynchronous read, see AsyncReadContext.
/// @note Requires open62541 v1.5 or later and `UA_MULTITHREADING >= 100`.
template <>
class DataSource<true, false> : public DataSourceAdapter {
public:
    /**
     * Callback to set the read value, the result status and optionally a source timestamp.
     * To complete the read later from any thread, call `ctx.defer()` and return the status it
     * provides, see AsyncReadContext.
     *
     * @param session Current session
     * @param id The identifier of the node being read from
     * @param range If not `nullptr`, then the data source shall return only a selection of the
     *              (non-scalar) data.
     *              Set `UA_STATUSCODE_BADINDEXRANGEINVALID` in `value` if this does not apply.
     * @param value The DataValue that is returned to the reader
     * @param timestamp Set the source timestamp of `value` if `true`
     * @param ctx Context to defer the completion of the read
     * @return StatusCode
     */
    virtual StatusCode read(
        Session& session,
        const NodeId& id,
        const NumericRange* range,
        DataValue& value,
        bool timestamp,
        AsyncReadContext& ctx
    ) = 0;

    /**
     * Callback to write the value into a data source.
     *
     * @param session Current session
     * @param id The identifier of the node being written to
     * @param range If not `nullptr`, then only this selection of (non-scalar) data should be
     *              written into the data source.
     * @param value The DataValue that has been written by the writer
     * @return StatusCode
     */
    virtual StatusCode write(
        Session& session, const NodeId& id, const NumericRange* range, const DataValue& value
    ) = 0;

    detail::NativeCallbackValueSource create(bool ownsAdapter) override {
        if (ownsAdapter) {
            throw BadStatus(UA_STATUSCODE_BADINTERNALERROR);
        }
        detail::NativeCallbackValueSource native{};
        native.read = &readNative;
        native.write = &writeNative;
        return native;
    }

private:
    static UA_StatusCode readNative(
        UA_Server* server,
        const UA_NodeId* sessionId,
        void* sessionContext,
        const UA_NodeId* nodeId,
        void* nodeContext,
        UA_Boolean includeSourceTimeStamp,
        const UA_NumericRange* range,
        UA_DataValue* value
    ) noexcept {
        return detail::dataSourceReadNativeAsync<DataSource<true, false>>(
            server,
            sessionId,
            sessionContext,
            nodeId,
            nodeContext,
            includeSourceTimeStamp,
            range,
            value
        );
    }

    static UA_StatusCode writeNative(
        UA_Server* server,
        const UA_NodeId* sessionId,
        void* sessionContext,
        const UA_NodeId* nodeId,
        void* nodeContext,
        const UA_NumericRange* range,
        const UA_DataValue* value
    ) noexcept {
        return detail::dataSourceWriteNativeSync<DataSource<true, false>>(
            server, sessionId, sessionContext, nodeId, nodeContext, range, value
        );
    }
};

/// Data source with asynchronous write, see AsyncWriteContext.
/// @note Requires open62541 v1.5 or later and `UA_MULTITHREADING >= 100`.
template <>
class DataSource<false, true> : public DataSourceAdapter {
public:
    /**
     * Callback to set the read value, the result status and optionally a source timestamp.
     *
     * @note Zero-copy operations are possible. It is not required to return a copy of the actual
     * content data. You can return a pointer to memory owned by the user. Memory can be reused
     * between read callbacks of a data source, as the result is already encoded on the network
     * buffer between each read operation.
     * To use zero-copy reads, set the value of the Variant (DataValue::setValue) without copying,
     * e.g. with Variant::assign.
     *
     * @param session Current session
     * @param id The identifier of the node being read from
     * @param range If not `nullptr`, then the data source shall return only a selection of the
     *              (non-scalar) data.
     *              Set `UA_STATUSCODE_BADINDEXRANGEINVALID` in `value` if this does not apply.
     * @param value The DataValue that is returned to the reader
     * @param timestamp Set the source timestamp of `value` if `true`
     * @return StatusCode
     */
    virtual StatusCode read(
        Session& session,
        const NodeId& id,
        const NumericRange* range,
        DataValue& value,
        bool timestamp
    ) = 0;

    /**
     * Callback to write the value into a data source.
     * To complete the write later from any thread, call `ctx.defer()` and return the status it
     * provides, see AsyncWriteContext.
     *
     * @param session Current session
     * @param id The identifier of the node being written to
     * @param range If not `nullptr`, then only this selection of (non-scalar) data should be
     *              written into the data source.
     * @param value The DataValue that has been written by the writer
     * @param ctx Context to defer the completion of the write
     * @return StatusCode
     */
    virtual StatusCode write(
        Session& session,
        const NodeId& id,
        const NumericRange* range,
        const DataValue& value,
        AsyncWriteContext& ctx
    ) = 0;

    detail::NativeCallbackValueSource create(bool ownsAdapter) override {
        if (ownsAdapter) {
            throw BadStatus(UA_STATUSCODE_BADINTERNALERROR);
        }
        detail::NativeCallbackValueSource native{};
        native.read = &readNative;
        native.write = &writeNative;
        return native;
    }

private:
    static UA_StatusCode readNative(
        UA_Server* server,
        const UA_NodeId* sessionId,
        void* sessionContext,
        const UA_NodeId* nodeId,
        void* nodeContext,
        UA_Boolean includeSourceTimeStamp,
        const UA_NumericRange* range,
        UA_DataValue* value
    ) noexcept {
        return detail::dataSourceReadNativeSync<DataSource<false, true>>(
            server,
            sessionId,
            sessionContext,
            nodeId,
            nodeContext,
            includeSourceTimeStamp,
            range,
            value
        );
    }

    static UA_StatusCode writeNative(
        UA_Server* server,
        const UA_NodeId* sessionId,
        void* sessionContext,
        const UA_NodeId* nodeId,
        void* nodeContext,
        const UA_NumericRange* range,
        const UA_DataValue* value
    ) noexcept {
        return detail::dataSourceWriteNativeAsync<DataSource<false, true>>(
            server, sessionId, sessionContext, nodeId, nodeContext, range, value
        );
    }
};

/// Data source with asynchronous read and write, see AsyncReadContext and AsyncWriteContext.
/// @note Requires open62541 v1.5 or later and `UA_MULTITHREADING >= 100`.
template <>
class DataSource<true, true> : public DataSourceAdapter {
public:
    /**
     * Callback to set the read value, the result status and optionally a source timestamp.
     * To complete the read later from any thread, call `ctx.defer()` and return the status it
     * provides, see AsyncReadContext.
     *
     * @param session Current session
     * @param id The identifier of the node being read from
     * @param range If not `nullptr`, then the data source shall return only a selection of the
     *              (non-scalar) data.
     *              Set `UA_STATUSCODE_BADINDEXRANGEINVALID` in `value` if this does not apply.
     * @param value The DataValue that is returned to the reader
     * @param timestamp Set the source timestamp of `value` if `true`
     * @param ctx Context to defer the completion of the read
     * @return StatusCode
     */
    virtual StatusCode read(
        Session& session,
        const NodeId& id,
        const NumericRange* range,
        DataValue& value,
        bool timestamp,
        AsyncReadContext& ctx
    ) = 0;

    /**
     * Callback to write the value into a data source.
     * To complete the write later from any thread, call `ctx.defer()` and return the status it
     * provides, see AsyncWriteContext.
     *
     * @param session Current session
     * @param id The identifier of the node being written to
     * @param range If not `nullptr`, then only this selection of (non-scalar) data should be
     *              written into the data source.
     * @param value The DataValue that has been written by the writer
     * @param ctx Context to defer the completion of the write
     * @return StatusCode
     */
    virtual StatusCode write(
        Session& session,
        const NodeId& id,
        const NumericRange* range,
        const DataValue& value,
        AsyncWriteContext& ctx
    ) = 0;

    detail::NativeCallbackValueSource create(bool ownsAdapter) override {
        if (ownsAdapter) {
            throw BadStatus(UA_STATUSCODE_BADINTERNALERROR);
        }
        detail::NativeCallbackValueSource native{};
        native.read = &readNative;
        native.write = &writeNative;
        return native;
    }

private:
    static UA_StatusCode readNative(
        UA_Server* server,
        const UA_NodeId* sessionId,
        void* sessionContext,
        const UA_NodeId* nodeId,
        void* nodeContext,
        UA_Boolean includeSourceTimeStamp,
        const UA_NumericRange* range,
        UA_DataValue* value
    ) noexcept {
        return detail::dataSourceReadNativeAsync<DataSource<true, true>>(
            server,
            sessionId,
            sessionContext,
            nodeId,
            nodeContext,
            includeSourceTimeStamp,
            range,
            value
        );
    }

    static UA_StatusCode writeNative(
        UA_Server* server,
        const UA_NodeId* sessionId,
        void* sessionContext,
        const UA_NodeId* nodeId,
        void* nodeContext,
        const UA_NumericRange* range,
        const UA_DataValue* value
    ) noexcept {
        return detail::dataSourceWriteNativeAsync<DataSource<true, true>>(
            server, sessionId, sessionContext, nodeId, nodeContext, range, value
        );
    }
};

}  // namespace opcua

#endif  // UAPP_HAS_ASYNC_OPERATIONS && UAPP_OPEN62541_VER_GE(1, 5)
