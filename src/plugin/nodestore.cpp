#include "open62541pp/plugin/nodestore.hpp"

#include <cassert>
#include <optional>

#include "open62541pp/detail/result_utils.hpp"
#include "open62541pp/detail/server_context.hpp"
#include "open62541pp/detail/server_utils.hpp"
#include "open62541pp/exception.hpp"
#include "open62541pp/server.hpp"

namespace opcua::detail {

DataSourceAdapter* getDataSource(void* nodeContext) noexcept {
    assert(nodeContext != nullptr);
    return static_cast<NodeContext*>(nodeContext)->dataSource.get();
}

#if UAPP_HAS_ASYNC_OPERATIONS && UAPP_OPEN62541_VER_GE(1, 5)
AsyncOperationRegistry& getAsyncOperations(UA_Server* server) noexcept {
    return getContext(*asWrapper(server)).asyncOperations;
}
#endif

}  // namespace opcua::detail

namespace opcua {

static void onReadNative(
    UA_Server* server,
    const UA_NodeId* sessionId,
    void* sessionContext,
    const UA_NodeId* nodeId,
    void* nodeContext,
    const UA_NumericRange* range,
    const UA_DataValue* value
) noexcept {
    assert(nodeContext != nullptr && nodeId != nullptr && value != nullptr);
    auto& callback = static_cast<detail::NodeContext*>(nodeContext)->valueCallback;
    auto* catcher = detail::getExceptionCatcher(server);
    auto session = detail::getSession(server, sessionId, sessionContext);
    if (callback != nullptr && catcher != nullptr && session) {
        catcher->invoke([&] {
            callback->onRead(
                session.value(),
                asWrapper<NodeId>(*nodeId),
                asWrapper<NumericRange>(range),
                asWrapper<DataValue>(*value)
            );
        });
    }
}

static void onWriteNative(
    UA_Server* server,
    const UA_NodeId* sessionId,
    void* sessionContext,
    const UA_NodeId* nodeId,
    void* nodeContext,
    const UA_NumericRange* range,
    const UA_DataValue* value
) noexcept {
    assert(nodeContext != nullptr && nodeId != nullptr && value != nullptr);
    auto& callback = static_cast<detail::NodeContext*>(nodeContext)->valueCallback;
    auto* catcher = detail::getExceptionCatcher(server);
    auto session = detail::getSession(server, sessionId, sessionContext);
    if (callback != nullptr && catcher != nullptr && session) {
        catcher->invoke([&] {
            callback->onWrite(
                session.value(),
                asWrapper<NodeId>(*nodeId),
                asWrapper<NumericRange>(range),
                asWrapper<DataValue>(*value)
            );
        });
    }
}

detail::NativeValueSourceNotifications ValueCallbackBase::create(bool ownsAdapter) {
    if (ownsAdapter) {
        throw BadStatus(UA_STATUSCODE_BADINTERNALERROR);
    }
    detail::NativeValueSourceNotifications native{};
    native.onRead = onReadNative;
    native.onWrite = onWriteNative;
    return native;
}

}  // namespace opcua
