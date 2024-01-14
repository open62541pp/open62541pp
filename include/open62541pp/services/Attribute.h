#pragma once

#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

#include "open62541pp/Client.h"
#include "open62541pp/Common.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Span.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/async.h"
#include "open62541pp/open62541.h"
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
 *
 * The following node attributes cannot be changed once a node has been created:
 * - NodeClass
 * - NodeId
 * - Symmetric
 * - ContainsNoLoops
 *
 * The following attributes cannot be written from the server, as they are specific to the different
 * users and set by the access control callback:
 * - UserWriteMask
 * - UserAccessLevel
 * - UserExecutable
 *
 * @see https://www.open62541.org/doc/1.3/server.html
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10
 * @ingroup Services
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
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readAsync(
    Client& client, const ReadRequest& request, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<UA_ReadRequest, UA_ReadResponse>(
        client, request, detail::MoveResponse{}, std::forward<CompletionToken>(token)
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
 */
template <typename CompletionToken = DefaultCompletionToken>
WriteResponse writeAsync(
    Client& client, const WriteRequest& request, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<UA_WriteRequest, UA_WriteResponse>(
        client, request, detail::MoveResponse{}, std::forward<CompletionToken>(token)
    );
}

/**
 * @overload
 */
template <typename CompletionToken = DefaultCompletionToken>
inline WriteResponse writeAsync(
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
 */
template <typename CompletionToken = DefaultCompletionToken>
void writeAttributeAsync(
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
        [](UA_WriteResponse& response) {
            opcua::detail::throwOnBadStatus(detail::getSingleResult(response));
        },
        std::forward<CompletionToken>(token)
    );
}

/* -------------------------------- Specialized inline functions -------------------------------- */

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

}  // namespace detail

/**
 * Read the `NodeId` attribute of a node.
 *
 * This function is mainly used to check the existence of a node.
 */
template <typename T>
inline NodeId readNodeId(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::NodeId>(serverOrClient, id);
}

template <typename CompletionToken = DefaultCompletionToken>
inline auto readNodeIdAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::NodeId>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the `NodeClass` attribute of a node.
 */
template <typename T>
inline NodeClass readNodeClass(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::NodeClass>(serverOrClient, id);
}

/**
 * Read the `BrowseName` attribute of a node.
 *
 * A non-localised human-readable name used to browse the address space.
 * The browse name of a reference type must be unique in a server.
 */
template <typename T>
inline QualifiedName readBrowseName(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::BrowseName>(serverOrClient, id);
}

/**
 * Read the `DisplayName` attribute of a node.
 *
 * Used to display the node to the user.
 */
template <typename T>
inline LocalizedText readDisplayName(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::DisplayName>(serverOrClient, id);
}

/**
 * Read the `Description` attribute of a node.
 *
 * The description shall explain the meaning of the node.
 */
template <typename T>
inline LocalizedText readDescription(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::Description>(serverOrClient, id);
}

/**
 * Read the `WriteMask` attribute of a node.
 *
 * For example `::UA_WRITEMASK_VALUEFORVARIABLETYPE | ::UA_WRITEMASK_VALUERANK`.
 */
template <typename T>
inline uint32_t readWriteMask(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::WriteMask>(serverOrClient, id);
}

/**
 * Read the `UserWriteMask` attribute of a node.
 *
 * @copydetails readWriteMask
 * In contrast to the write mask, the user write mask is taking access rights into account.
 */
template <typename T>
inline uint32_t readUserWriteMask(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::UserWriteMask>(serverOrClient, id);
}

/**
 * Read the `IsAbtract` attribute of a reference type node.
 *
 * If a reference is abstract, no reference of this type shall exist, only of its subtypes.
 */
template <typename T>
inline bool readIsAbstract(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::IsAbstract>(serverOrClient, id);
}

/**
 * Read the `Symmetric` attribute of a reference type node.
 *
 * If a reference is symmetric it can seen from both the source and target node.
 */
template <typename T>
inline bool readSymmetric(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::Symmetric>(serverOrClient, id);
}

/**
 * Read the `InverseName` attribute of a reference type node.
 *
 * The inverse name describes the reference type as seen from the target node.
 * For example, the reference type `HasSubtype` has the inverse name `HasSupertype`.
 */
template <typename T>
inline LocalizedText readInverseName(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::InverseName>(serverOrClient, id);
}

/**
 * Read the `Value` attribute of a variable node as a DataValue object.
 */
template <typename T>
inline DataValue readDataValue(T& serverOrClient, const NodeId& id) {
    return readAttribute(serverOrClient, id, AttributeId::Value, TimestampsToReturn::Both);
}

/// @copydoc readDataValue
template <typename T>
[[deprecated("No performance benefit to pass DataValue by reference, return by value instead"
)]] inline void
readDataValue(T& serverOrClient, const NodeId& id, DataValue& value) {
    value = readDataValue(serverOrClient, id);
}

/**
 * Read the `Value` attribute of a variable node as a Variant object.
 */
template <typename T>
inline Variant readValue(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::Value>(serverOrClient, id);
}

/// @copydoc readValue
template <typename T>
[[deprecated("No performance benefit to pass Variant by reference, return by value instead."
)]] inline void
readValue(T& serverOrClient, const NodeId& id, Variant& value) {
    value = readValue(serverOrClient, id);
}

/**
 * Read the `DataType` attribute of a variable (type) node as NodeId.
 */
template <typename T>
inline NodeId readDataType(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::DataType>(serverOrClient, id);
}

/**
 * Read the `ValueRank` attribute of a variable (type) node.
 *
 * Indicates whether the value attribute of the variable node is an array and how many dimensions
 * the array has.
 */
template <typename T>
inline ValueRank readValueRank(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::ValueRank>(serverOrClient, id);
}

/**
 * Read the `ArrayDimensions` attribute of a variable (type) node.
 *
 * Specifies the maximum supported length of each dimension (0 if unknown).
 */
template <typename T>
inline std::vector<uint32_t> readArrayDimensions(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::ArrayDimensions>(serverOrClient, id);
}

/**
 * Read the `AccessLevel` attribute of a variable node.
 *
 * For example `::UA_ACCESSLEVELMASK_READ | ::UA_ACCESSLEVELMASK_WRITE`.
 * The access level attribute is used to indicate how the value of a variable can be accessed
 * (read/write) and if it contains current and/or historic data.
 */
template <typename T>
inline uint8_t readAccessLevel(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::AccessLevel>(serverOrClient, id);
}

/**
 * Read the `UserAccessLevel` attribute a variable node.
 *
 * @copydetails readAccessLevel
 * In contrast to the access level, the user access level is taking access rights into account.
 */
template <typename T>
inline uint8_t readUserAccessLevel(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::UserAccessLevel>(serverOrClient, id);
}

/**
 * Read the `MinimumSamplingInterval` attribute of a variable node.
 *
 * Specifies (in milliseconds) how fast the server can reasonably sample the value for changes.
 * A minimum sampling interval of 0 indicates that the server is to monitor the item continuously.
 * A minimum sampling interval of -1 means indeterminate.
 */
template <typename T>
inline double readMinimumSamplingInterval(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::MinimumSamplingInterval>(serverOrClient, id);
}

/* ---------------------------------------------------------------------------------------------- */

/**
 * Write the `DisplayName` attribute of a node.
 * @copydetails readDisplayName
 */
template <typename T>
inline void writeDisplayName(T& serverOrClient, const NodeId& id, const LocalizedText& name) {
    writeAttribute(serverOrClient, id, AttributeId::DisplayName, DataValue::fromScalar(name));
}

/**
 * Write the `Description` attribute of a node.
 * @copydetails readDescription
 */
template <typename T>
inline void writeDescription(T& serverOrClient, const NodeId& id, const LocalizedText& desc) {
    writeAttribute(serverOrClient, id, AttributeId::Description, DataValue::fromScalar(desc));
}

/**
 * Write the `WriteMask` attribute of a node.
 * @copydetails readWriteMask
 */
template <typename T>
inline void writeWriteMask(T& serverOrClient, const NodeId& id, uint32_t mask) {
    writeAttribute(serverOrClient, id, AttributeId::WriteMask, DataValue::fromScalar(mask));
}

/**
 * Write the `UserWriteMask` attribute of a node.
 * @copydetails readUserWriteMask
 * @note Cannot be written from the server.
 */
template <typename T>
inline void writeUserWriteMask(T& serverOrClient, const NodeId& id, uint32_t mask) {
    writeAttribute(serverOrClient, id, AttributeId::UserWriteMask, DataValue::fromScalar(mask));
}

/**
 * Write the `IsAbstract` attribute of a reference type node.
 * @copydetails readIsAbstract
 */
template <typename T>
inline void writeIsAbstract(T& serverOrClient, const NodeId& id, bool isAbstract) {
    writeAttribute(serverOrClient, id, AttributeId::IsAbstract, DataValue::fromScalar(isAbstract));
}

/**
 * Write the `Symmetric` attribute of a reference type node.
 * @copydetails readSymmetric
 */
template <typename T>
inline void writeSymmetric(T& serverOrClient, const NodeId& id, bool symmetric) {
    writeAttribute(serverOrClient, id, AttributeId::Symmetric, DataValue::fromScalar(symmetric));
}

/**
 * Write the `InverseName` of a reference type node.
 * @copydetails readInverseName
 */
template <typename T>
inline void writeInverseName(T& serverOrClient, const NodeId& id, const LocalizedText& name) {
    writeAttribute(serverOrClient, id, AttributeId::InverseName, DataValue::fromScalar(name));
}

/**
 * Write the `Value` attribute of a variable node as a DataValue object.
 * @copydetails readDataValue
 */
template <typename T>
inline void writeDataValue(T& serverOrClient, const NodeId& id, const DataValue& value) {
    writeAttribute(serverOrClient, id, AttributeId::Value, value);
}

/**
 * Write the `Value` attribute of a variable node as a Variant object.
 * @copydetails readValue
 */
template <typename T>
inline void writeValue(T& serverOrClient, const NodeId& id, const Variant& value) {
    UA_DataValue dv{};
    dv.value = *value.handle();  // shallow copy
    dv.hasValue = true;
    writeAttribute(serverOrClient, id, AttributeId::Value, asWrapper<DataValue>(dv));
}

/**
 * Write the `DataType` attribute of a variable (type) node.
 * @copydetails readDataType
 */
template <typename T>
inline void writeDataType(T& serverOrClient, const NodeId& id, const NodeId& typeId) {
    writeAttribute(serverOrClient, id, AttributeId::DataType, DataValue::fromScalar(typeId));
}

/**
 * Write the `ValueRank` attribute of a variable (type) node.
 * @copydetails readValueRank
 */
template <typename T>
inline void writeValueRank(T& serverOrClient, const NodeId& id, ValueRank valueRank) {
    const auto native = static_cast<int32_t>(valueRank);
    writeAttribute(serverOrClient, id, AttributeId::ValueRank, DataValue::fromScalar(native));
}

/**
 * Write the `ArrayDimensions` attribute of a variable (type) node.
 *
 * Should be unspecified if ValueRank is <= 0 (ValueRank::Any, ValueRank::Scalar,
 * ValueRank::ScalarOrOneDimension, ValueRank::OneOrMoreDimensions). The dimension zero is a
 * wildcard and the actual value may have any length in this dimension.
 * @copydetails readArrayDimensions
 */
template <typename T>
inline void writeArrayDimensions(
    T& serverOrClient, const NodeId& id, Span<const uint32_t> dimensions
) {
    writeAttribute(
        serverOrClient, id, AttributeId::ArrayDimensions, DataValue::fromArray(dimensions)
    );
}

/**
 * Write the `AccessLevel` attribute of a variable node.
 * @copydetails readAccessLevel
 */
template <typename T>
inline void writeAccessLevel(T& serverOrClient, const NodeId& id, uint8_t mask) {
    writeAttribute(
        serverOrClient,
        id,
        AttributeId::AccessLevel,
        DataValue::fromScalar(static_cast<UA_Byte>(mask))
    );
}

/**
 * Write the `UserAccessLevel` attribute of a variable node.
 * @copydetails readUserAccessLevel
 * @note Cannot be written from the server.
 */
template <typename T>
inline void writeUserAccessLevel(T& serverOrClient, const NodeId& id, uint8_t mask) {
    writeAttribute(
        serverOrClient,
        id,
        AttributeId::UserAccessLevel,
        DataValue::fromScalar(static_cast<UA_Byte>(mask))
    );
}

/**
 * Write the `MinimumSamplingInterval` attribute of a variable node.
 * @copydetails readMinimumSamplingInterval
 */
template <typename T>
inline void writeMinimumSamplingInterval(T& serverOrClient, const NodeId& id, double milliseconds) {
    writeAttribute(
        serverOrClient,
        id,
        AttributeId::MinimumSamplingInterval,
        DataValue::fromScalar(milliseconds)
    );
}

/**
 * @}
 */

}  // namespace opcua::services
