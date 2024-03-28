#pragma once

#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

#include "open62541pp/Bitmask.h"
#include "open62541pp/Client.h"
#include "open62541pp/Common.h"  // TimestampsToReturn
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Span.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/async.h"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/services/detail/AttributeHandler.h"
#include "open62541pp/services/detail/ClientService.h"
#include "open62541pp/services/detail/RequestHandling.h"
#include "open62541pp/services/detail/ResponseHandling.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

namespace opcua::services {

/* -------------------------------------- Generic functions ------------------------------------- */

/**
 * @defgroup Attribute Attribute service set
 * Read and write node attributes.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10
 * @ingroup Services
 * @{
 */

/**
 * @defgroup Read Read service
 * This service is used to read attributes of nodes.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10.2
 * @{
 */

/**
 * Read one or more attributes of one or more nodes (client only).
 * @param connection Instance of type Client
 * @param request Read request
 */
ReadResponse read(Client& connection, const ReadRequest& request);

/**
 * @overload
 */
inline ReadResponse read(
    Client& connection,
    Span<const ReadValueId> nodesToRead,
    TimestampsToReturn timestamps = TimestampsToReturn::Neither
) {
    return read(connection, detail::createReadRequest(timestamps, nodesToRead));
}

/**
 * Asynchronously read one or more attributes of one or more nodes (client only).
 * @copydetails read
 * @param token @completiontoken{void(Result<ReadResponse>&)}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readAsync(
    Client& connection,
    const ReadRequest& request,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<UA_ReadRequest, UA_ReadResponse>(
        connection,
        request,
        detail::WrapResponse<ReadResponse>{},
        std::forward<CompletionToken>(token)
    );
}

/**
 * @overload
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readAsync(
    Client& connection,
    Span<const ReadValueId> nodesToRead,
    TimestampsToReturn timestamps = TimestampsToReturn::Neither,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return readAsync(
        connection,
        detail::createReadRequest(timestamps, nodesToRead),
        std::forward<CompletionToken>(token)
    );
}

/**
 * Read node attribute.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @param attributeId Attribute to read
 * @param timestamps Timestamps to return
 */
template <typename T>
DataValue readAttribute(
    T& connection,
    const NodeId& id,
    AttributeId attributeId,
    TimestampsToReturn timestamps = TimestampsToReturn::Neither
);

/**
 * Asynchronously read node attribute.
 * @copydetails readAttribute
 * @param token @completiontoken{void(Result<DataValue>&)}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readAttributeAsync(
    Client& connection,
    const NodeId& id,
    AttributeId attributeId,
    TimestampsToReturn timestamps = TimestampsToReturn::Neither,
    CompletionToken&& token = DefaultCompletionToken()
) {
    auto item = detail::createReadValueId(id, attributeId);
    auto request = detail::createReadRequest(timestamps, item);
    return detail::sendRequest<UA_ReadRequest, UA_ReadResponse>(
        connection,
        request,
        [](UA_ReadResponse& response) { return detail::getReadResult(response); },
        std::forward<CompletionToken>(token)
    );
}

/**
 * @}
 * @defgroup Write Write service
 * This service is used to write attributes of nodes.
 *
 * The following node attributes cannot be changed once a node has been created:
 * - AttributeId::NodeClass
 * - AttributeId::NodeId
 * - AttributeId::Symmetric
 * - AttributeId::ContainsNoLoops
 *
 * The following attributes cannot be written from the server, as they are specific to the different
 * users and set by the access control callback:
 * - AttributeId::UserWriteMask
 * - AttributeId::UserAccessLevel
 * - AttributeId::UserExecutable
 *
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10.4
 * @{
 */

/**
 * Write one or more attributes of one or more nodes (client only).
 * @param connection Instance of type Client
 * @param request Write request
 */
WriteResponse write(Client& connection, const WriteRequest& request);

/**
 * @overload
 */
inline WriteResponse write(Client& connection, Span<const WriteValue> nodesToWrite) {
    return write(connection, detail::createWriteRequest(nodesToWrite));
}

/**
 * Asynchronously write one or more attributes of one or more nodes (client only).
 * @copydetails write
 * @param token @completiontoken{void(Result<WriteResponse>&)}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeAsync(
    Client& connection,
    const WriteRequest& request,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<UA_WriteRequest, UA_WriteResponse>(
        connection,
        request,
        detail::WrapResponse<WriteResponse>{},
        std::forward<CompletionToken>(token)
    );
}

/**
 * @overload
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeAsync(
    Client& connection,
    Span<const WriteValue> nodesToWrite,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return writeAsync(
        connection, detail::createWriteRequest(nodesToWrite), std::forward<CompletionToken>(token)
    );
}

/**
 * Write node attribute.
 * @param connection Instance of type Client
 * @param id Node to write
 * @param attributeId Attribute to write
 * @param value Value to write
 */
template <typename T>
void writeAttribute(
    T& connection, const NodeId& id, AttributeId attributeId, const DataValue& value
);

/**
 * Asynchronously write node attribute.
 * @copydetails writeAttribute
 * @param token @completiontoken{void(Result<void>)}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeAttributeAsync(
    Client& connection,
    const NodeId& id,
    AttributeId attributeId,
    const DataValue& value,
    CompletionToken&& token = DefaultCompletionToken()
) {
    auto item = detail::createWriteValue(id, attributeId, value);
    auto request = detail::createWriteRequest(item);
    return detail::sendRequest<UA_WriteRequest, UA_WriteResponse>(
        connection,
        request,
        [](UA_WriteResponse& response) { throwIfBad(detail::getSingleResult(response)); },
        std::forward<CompletionToken>(token)
    );
}

/**
 * @}
 */

/* ----------------------------- Specializated inline read functions ---------------------------- */

namespace detail {

template <AttributeId Attribute, typename T>
inline auto readAttributeImpl(T& connection, const NodeId& id) {
    using Handler = typename detail::AttributeHandler<Attribute>;
    return Handler::fromDataValue(readAttribute(connection, id, Attribute));
}

template <AttributeId Attribute, typename CompletionToken>
inline auto readAttributeAsyncImpl(Client& connection, const NodeId& id, CompletionToken&& token) {
    auto item = detail::createReadValueId(id, Attribute);
    auto request = detail::createReadRequest(TimestampsToReturn::Neither, item);
    return detail::sendRequest<UA_ReadRequest, UA_ReadResponse>(
        connection,
        request,
        [](UA_ReadResponse& response) {
            using Handler = typename detail::AttributeHandler<Attribute>;
            return Handler::fromDataValue(detail::getReadResult(response));
        },
        std::forward<CompletionToken>(token)
    );
}

template <AttributeId Attribute, typename T, typename U>
inline void writeAttributeImpl(T& connection, const NodeId& id, U&& value) {
    using Handler = detail::AttributeHandler<Attribute>;
    writeAttribute(connection, id, Attribute, Handler::toDataValue(std::forward<U>(value)));
}

template <AttributeId Attribute, typename T, typename U, typename CompletionToken>
inline auto writeAttributeAsyncImpl(
    T& connection, const NodeId& id, U&& value, CompletionToken&& token
) {
    using Handler = detail::AttributeHandler<Attribute>;
    return writeAttributeAsync(
        connection,
        id,
        Attribute,
        Handler::toDataValue(std::forward<U>(value)),
        std::forward<CompletionToken>(token)
    );
}

}  // namespace detail

/**
 * Read the `AttributeId::Value` attribute of a node as a DataValue object.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
inline DataValue readDataValue(T& connection, const NodeId& id) {
    return readAttribute(connection, id, AttributeId::Value, TimestampsToReturn::Both);
}

/**
 * Asynchronously read the `AttributeId::Value` of a node as a DataValue object.
 * @copydetails readDataValue
 * @param token @completiontoken{void(Result<DataValue>&)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readDataValueAsync(Client& connection, const NodeId& id, CompletionToken&& token) {
    return readAttributeAsync(
        connection,
        id,
        AttributeId::Value,
        TimestampsToReturn::Both,
        std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::Value attribute of a node as a DataValue object.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param value Value to write
 * @ingroup Write
 */
template <typename T>
inline void writeDataValue(T& connection, const NodeId& id, const DataValue& value) {
    writeAttribute(connection, id, AttributeId::Value, value);
}

/**
 * Asynchronously write the AttributeId::Value attribute of a node as a DataValue object.
 * @copydetails writeDataValue
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeDataValueAsync(
    Client& connection,
    const NodeId& id,
    const DataValue& value,
    CompletionToken&& token = DefaultCompletionToken()
) {
    writeAttributeAsync(
        connection, id, AttributeId::Value, value, std::forward<CompletionToken>(token)
    );
}

/**
 * @}
 */

}  // namespace opcua::services

#include "open62541pp/services/Attribute_highlevel.h"
