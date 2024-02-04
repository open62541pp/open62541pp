#pragma once

#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

#include "open62541pp/Bitmask.h"
#include "open62541pp/Client.h"
#include "open62541pp/Common.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Span.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/async.h"
#include "open62541pp/open62541.h"
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
 * @defgroup Read
 * This service is used to read attributes of nodes.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10.2
 * @{
 */

/**
 * Read one or more attributes of one or more nodes (client only).
 */
ReadResponse read(Client& client, const ReadRequest& request);

/**
 * @overload
 */
inline ReadResponse read(
    Client& client,
    Span<const ReadValueId> nodesToRead,
    TimestampsToReturn timestamps = TimestampsToReturn::Neither
) {
    return read(client, detail::createReadRequest(timestamps, nodesToRead));
}

/**
 * Asynchronously read one or more attributes of one or more nodes (client only).
 * @copydetails read
 * @param token @completiontoken{void(opcua::StatusCode, opcua::ReadResponse&)}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readAsync(
    Client& client, const ReadRequest& request, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<UA_ReadRequest, UA_ReadResponse>(
        client, request, detail::WrapResponse<ReadResponse>{}, std::forward<CompletionToken>(token)
    );
}

/**
 * @overload
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readAsync(
    Client& client,
    Span<const ReadValueId> nodesToRead,
    TimestampsToReturn timestamps = TimestampsToReturn::Neither,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return readAsync(
        client,
        detail::createReadRequest(timestamps, nodesToRead),
        std::forward<CompletionToken>(token)
    );
}

/**
 * Read node attribute.
 */
template <typename T>
DataValue readAttribute(
    T& serverOrClient,
    const NodeId& id,
    AttributeId attributeId,
    TimestampsToReturn timestamps = TimestampsToReturn::Neither
);

/**
 * Asynchronously read node attribute.
 * @copydetails readAttribute
 * @param token @completiontoken{void(opcua::StatusCode, opcua::DataValue&)}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readAttributeAsync(
    Client& client,
    const NodeId& id,
    AttributeId attributeId,
    TimestampsToReturn timestamps = TimestampsToReturn::Neither,
    CompletionToken&& token = DefaultCompletionToken()
) {
    auto item = detail::createReadValueId(id, attributeId);
    auto request = detail::createReadRequest(timestamps, item);
    return detail::sendRequest<UA_ReadRequest, UA_ReadResponse>(
        client,
        request,
        [](UA_ReadResponse& response) { return detail::getReadResult(response); },
        std::forward<CompletionToken>(token)
    );
}

/**
 * @}
 * @defgroup Write
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
 */
WriteResponse write(Client& client, const WriteRequest& request);

/**
 * @overload
 */
inline WriteResponse write(Client& client, Span<const WriteValue> nodesToWrite) {
    return write(client, detail::createWriteRequest(nodesToWrite));
}

/**
 * Asynchronously write one or more attributes of one or more nodes (client only).
 * @copydetails write
 * @param token @completiontoken{void(opcua::StatusCode, opcua::WriteResponse&)}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeAsync(
    Client& client, const WriteRequest& request, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<UA_WriteRequest, UA_WriteResponse>(
        client, request, detail::WrapResponse<WriteResponse>{}, std::forward<CompletionToken>(token)
    );
}

/**
 * @overload
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeAsync(
    Client& client,
    Span<const WriteValue> nodesToWrite,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return writeAsync(
        client, detail::createWriteRequest(nodesToWrite), std::forward<CompletionToken>(token)
    );
}

/**
 * Write node attribute.
 */
template <typename T>
void writeAttribute(
    T& serverOrClient, const NodeId& id, AttributeId attributeId, const DataValue& value
);

/**
 * Asynchronously write node attribute.
 * @copydetails writeAttribute
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeAttributeAsync(
    Client& client,
    const NodeId& id,
    AttributeId attributeId,
    const DataValue& value,
    CompletionToken&& token = DefaultCompletionToken()
) {
    auto item = detail::createWriteValue(id, attributeId, value);
    auto request = detail::createWriteRequest(item);
    return detail::sendRequest<UA_WriteRequest, UA_WriteResponse>(
        client,
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
inline auto readAttributeImpl(T& serverOrClient, const NodeId& id) {
    using Handler = typename detail::AttributeHandler<Attribute>;
    return Handler::fromDataValue(readAttribute(serverOrClient, id, Attribute));
}

template <AttributeId Attribute, typename CompletionToken>
inline auto readAttributeAsyncImpl(Client& client, const NodeId& id, CompletionToken&& token) {
    auto item = detail::createReadValueId(id, Attribute);
    auto request = detail::createReadRequest(TimestampsToReturn::Neither, item);
    return detail::sendRequest<UA_ReadRequest, UA_ReadResponse>(
        client,
        request,
        [](UA_ReadResponse& response) {
            using Handler = typename detail::AttributeHandler<Attribute>;
            return Handler::fromDataValue(detail::getReadResult(response));
        },
        std::forward<CompletionToken>(token)
    );
}

template <AttributeId Attribute, typename T, typename U>
inline void writeAttributeImpl(T& serverOrClient, const NodeId& id, U&& value) {
    using Handler = detail::AttributeHandler<Attribute>;
    writeAttribute(serverOrClient, id, Attribute, Handler::toDataValue(std::forward<U>(value)));
}

template <AttributeId Attribute, typename T, typename U, typename CompletionToken>
inline auto writeAttributeAsyncImpl(
    T& serverOrClient, const NodeId& id, U&& value, CompletionToken&& token
) {
    using Handler = detail::AttributeHandler<Attribute>;
    return writeAttributeAsync(
        serverOrClient,
        id,
        Attribute,
        Handler::toDataValue(std::forward<U>(value)),
        std::forward<CompletionToken>(token)
    );
}

}  // namespace detail

/**
 * Read the `AttributeId::Value` attribute of a node as a DataValue object.
 * @ingroup Read
 */
template <typename T>
inline DataValue readDataValue(T& serverOrClient, const NodeId& id) {
    return readAttribute(serverOrClient, id, AttributeId::Value, TimestampsToReturn::Both);
}

/**
 * Asynchronously read the `AttributeId::Value` of a node as a DataValue object.
 * @param token @completiontoken{void(opcua::StatusCode, opcua::DataValue&)}
 * @ingroup Read
 */
template <typename T, typename CompletionToken = DefaultCompletionToken>
inline auto readDataValueAsync(T& serverOrClient, const NodeId& id, CompletionToken&& token) {
    return readAttributeAsync(
        serverOrClient,
        id,
        AttributeId::Value,
        TimestampsToReturn::Both,
        std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::Value attribute of a node as a DataValue object.
 * @ingroup Write
 */
template <typename T>
inline void writeDataValue(T& serverOrClient, const NodeId& id, const DataValue& value) {
    writeAttribute(serverOrClient, id, AttributeId::Value, value);
}

/**
 * Asynchronously write the AttributeId::Value attribute of a node as a DataValue object.
 * @param token @completiontoken{void(opcua::StatusCode)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeDataValueAsync(
    Client& client,
    const NodeId& id,
    const DataValue& value,
    CompletionToken&& token = DefaultCompletionToken()
) {
    writeAttributeAsync(
        client, id, AttributeId::Value, value, std::forward<CompletionToken>(token)
    );
}

/**
 * @}
 */

}  // namespace opcua::services

#include "open62541pp/services/Attribute_highlevel.h"
