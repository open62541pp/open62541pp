#pragma once

#include <utility>

#include "open62541pp/async.hpp"
#include "open62541pp/common.hpp"  // TimestampsToReturn
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/result.hpp"
#include "open62541pp/services/detail/async_transform.hpp"
#include "open62541pp/services/detail/attribute_handler.hpp"
#include "open62541pp/services/detail/client_service.hpp"
#include "open62541pp/services/detail/request_handling.hpp"
#include "open62541pp/services/detail/response_handling.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/types_composed.hpp"

namespace opcua {
class Client;
}  // namespace opcua

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
ReadResponse read(Client& connection, const ReadRequest& request) noexcept;

/**
 * @overload
 */
inline ReadResponse read(
    Client& connection,
    Span<const ReadValueId> nodesToRead,
    TimestampsToReturn timestamps = TimestampsToReturn::Neither
) noexcept {
    return read(connection, detail::createReadRequest(timestamps, nodesToRead));
}

/**
 * Asynchronously read one or more attributes of one or more nodes (client only).
 * @copydetails read
 * @param token @completiontoken{void(ReadResponse&)}
 * @return @asyncresult{ReadResponse}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readAsync(
    Client& connection,
    const ReadRequest& request,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequestAsync<ReadRequest, ReadResponse>(
        connection, request, std::forward<CompletionToken>(token)
    );
}

/**
 * @overload
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readAsync(
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
Result<DataValue> readAttribute(
    T& connection,
    const NodeId& id,
    AttributeId attributeId,
    TimestampsToReturn timestamps = TimestampsToReturn::Neither
) noexcept;

/**
 * Asynchronously read node attribute.
 * @copydetails readAttribute
 * @param token @completiontoken{void(Result<DataValue>&)}
 * @return @asyncresult{Result<DataValue>}
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
    const auto request = detail::createReadRequest(timestamps, item);
    return readAsync(
        connection,
        asWrapper<ReadRequest>(request),
        detail::TransformToken(
            detail::wrapSingleResult<DataValue, UA_ReadResponse>,
            std::forward<CompletionToken>(token)
        )
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
WriteResponse write(Client& connection, const WriteRequest& request) noexcept;

/**
 * @overload
 */
inline WriteResponse write(Client& connection, Span<const WriteValue> nodesToWrite) noexcept {
    return write(connection, detail::createWriteRequest(nodesToWrite));
}

/**
 * Asynchronously write one or more attributes of one or more nodes (client only).
 * @copydetails write
 * @param token @completiontoken{void(WriteResponse&)}
 * @return @asyncresult{WriteResponse}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeAsync(
    Client& connection,
    const WriteRequest& request,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequestAsync<WriteRequest, WriteResponse>(
        connection, request, std::forward<CompletionToken>(token)
    );
}

/**
 * @overload
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeAsync(
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
StatusCode writeAttribute(
    T& connection, const NodeId& id, AttributeId attributeId, const DataValue& value
) noexcept;

/**
 * Asynchronously write node attribute.
 * @copydetails writeAttribute
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
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
    const auto request = detail::createWriteRequest(item);
    return writeAsync(
        connection,
        asWrapper<WriteRequest>(request),
        detail::TransformToken(
            detail::getSingleStatus<UA_WriteResponse>, std::forward<CompletionToken>(token)
        )
    );
}

/**
 * @}
 */

/* --------------------------------- Specialized read functions --------------------------------- */

namespace detail {

template <AttributeId Attribute, typename T>
auto readAttributeImpl(T& connection, const NodeId& id) noexcept {
    using Handler = typename detail::AttributeHandler<Attribute>;
    return readAttribute(connection, id, Attribute).andThen(Handler::fromDataValue);
}

template <AttributeId Attribute, typename CompletionToken>
auto readAttributeAsyncImpl(Client& connection, const NodeId& id, CompletionToken&& token) {
    using Handler = typename detail::AttributeHandler<Attribute>;
    return readAttributeAsync(
        connection,
        id,
        Attribute,
        TimestampsToReturn::Neither,
        detail::TransformToken(
            [](Result<DataValue>& result) {
                return std::move(result).andThen(Handler::fromDataValue);
            },
            std::forward<CompletionToken>(token)
        )
    );
}

template <AttributeId Attribute, typename T, typename U>
StatusCode writeAttributeImpl(T& connection, const NodeId& id, U&& value) noexcept {
    using Handler = detail::AttributeHandler<Attribute>;
    return writeAttribute(connection, id, Attribute, Handler::toDataValue(std::forward<U>(value)));
}

template <AttributeId Attribute, typename T, typename U, typename CompletionToken>
auto writeAttributeAsyncImpl(T& connection, const NodeId& id, U&& value, CompletionToken&& token) {
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
Result<DataValue> readDataValue(T& connection, const NodeId& id) noexcept {
    return readAttribute(connection, id, AttributeId::Value, TimestampsToReturn::Both);
}

/**
 * Asynchronously read the `AttributeId::Value` of a node as a DataValue object.
 * @copydetails readDataValue
 * @param token @completiontoken{void(Result<DataValue>&)}
 * @return @asyncresult{Result<DataValue>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readDataValueAsync(Client& connection, const NodeId& id, CompletionToken&& token) {
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
StatusCode writeDataValue(T& connection, const NodeId& id, const DataValue& value) noexcept {
    return writeAttribute(connection, id, AttributeId::Value, value);
}

/**
 * Asynchronously write the AttributeId::Value attribute of a node as a DataValue object.
 * @copydetails writeDataValue
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeDataValueAsync(
    Client& connection,
    const NodeId& id,
    const DataValue& value,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return writeAttributeAsync(
        connection, id, AttributeId::Value, value, std::forward<CompletionToken>(token)
    );
}

/**
 * @}
 */

}  // namespace opcua::services
