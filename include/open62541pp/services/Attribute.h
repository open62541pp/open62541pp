#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "open62541pp/Common.h"
#include "open62541pp/Helper.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

// forward declarations
namespace opcua {
class Client;
class Server;
}  // namespace opcua

namespace opcua::services {

/* -------------------------------------- Generic functions ------------------------------------- */

/**
 * @defgroup Attribute Attribute
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
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs
 * @ingroup Services
 */

/**
 * Generic function to read node attributes.
 * @ingroup Attribute
 */
template <typename T>
DataValue readAttribute(
    T& serverOrClient,
    const NodeId& id,
    UA_AttributeId attributeId,
    UA_TimestampsToReturn timestamps = UA_TIMESTAMPSTORETURN_NEITHER
);

/// Helper function to read scalar node attributes.
template <typename AttributeType, typename T>
inline auto readAttributeScalar(T& serverOrClient, const NodeId& id, UA_AttributeId attributeId) {
    const auto dv = readAttribute(serverOrClient, id, attributeId);
    return dv.getValuePtr()->template getScalarCopy<AttributeType>();
}

/**
 * Generic function to write node attributes.
 * @ingroup Attribute
 */
template <typename T>
void writeAttribute(
    T& serverOrClient, const NodeId& id, UA_AttributeId attributeId, const DataValue& value
);

/* -------------------------------- Specialized inline functions -------------------------------- */

/**
 * Read the `NodeId` attribute of a node.
 *
 * This function is mainly used to check the existance of a node.
 * @ingroup Attribute
 */
template <typename T>
inline NodeId readNodeId(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<NodeId>(serverOrClient, id, UA_ATTRIBUTEID_NODEID);
}

/**
 * Read the `NodeClass` attribute of a node.
 * @ingroup Attribute
 */
template <typename T>
inline NodeClass readNodeClass(T& serverOrClient, const NodeId& id) {
    auto dv = readAttribute(serverOrClient, id, UA_ATTRIBUTEID_NODECLASS);
    // workaround to read enum from variant...
    return *static_cast<NodeClass*>(dv.getValuePtr()->handle()->data);
}

/**
 * Read the `BrowseName` attribute of a node.
 *
 * A non-localised human-readable name used to browse the address space.
 * The browse name of a reference type must be unique in a server.
 * @ingroup Attribute
 */
template <typename T>
inline std::string readBrowseName(T& serverOrClient, const NodeId& id) {
    const auto name = readAttributeScalar<QualifiedName>(
        serverOrClient, id, UA_ATTRIBUTEID_BROWSENAME
    );
    return std::string(name.getName());
}

/**
 * Read the `DisplayName` attribute of a node.
 *
 * Used to display the node to the user.
 * @ingroup Attribute
 */
template <typename T>
inline LocalizedText readDisplayName(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<LocalizedText>(serverOrClient, id, UA_ATTRIBUTEID_DISPLAYNAME);
}

/**
 * Read the `Description` attribute of a node.
 *
 * The description shall explain the meaning of the node.
 * @ingroup Attribute
 */
template <typename T>
inline LocalizedText readDescription(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<LocalizedText>(serverOrClient, id, UA_ATTRIBUTEID_DESCRIPTION);
}

/**
 * Read the `WriteMask` attribute of a node.
 *
 * For example `::UA_WRITEMASK_VALUEFORVARIABLETYPE | ::UA_WRITEMASK_VALUERANK`.
 * @ingroup Attribute
 */
template <typename T>
inline uint32_t readWriteMask(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<uint32_t>(serverOrClient, id, UA_ATTRIBUTEID_WRITEMASK);
}

/**
 * Read the `UserWriteMask` attribute of a node.
 *
 * @copydetails readWriteMask
 * In constrast to the write mask, the user write mask is taking access rights into account.
 * @ingroup Attribute
 */
template <typename T>
inline uint32_t readUserWriteMask(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<uint32_t>(serverOrClient, id, UA_ATTRIBUTEID_USERWRITEMASK);
}

/**
 * Read the `IsAbtract` attribute of a reference type node.
 *
 * If a reference is abstract, no reference of this type shall exist, only of its subtypes.
 * @ingroup Attribute
 */
template <typename T>
inline bool readIsAbstract(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<bool>(serverOrClient, id, UA_ATTRIBUTEID_ISABSTRACT);
}

/**
 * Read the `Symmetric` attribute of a reference type node.
 *
 * If a reference is symmetric it can seen from both the source and target node.
 * @ingroup Attribute
 */
template <typename T>
inline bool readSymmetric(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<bool>(serverOrClient, id, UA_ATTRIBUTEID_SYMMETRIC);
}

/**
 * Read the `InverseName` attribute of a reference type node.
 *
 * The inverse name describes the reference type as seen from the target node.
 * For example, the reference type `HasSubtype` has the inverse name `HasSupertype`.
 * @ingroup Attribute
 */
template <typename T>
inline LocalizedText readInverseName(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<LocalizedText>(serverOrClient, id, UA_ATTRIBUTEID_INVERSENAME);
}

/**
 * Read the `Value` attribute of a variable node as a DataValue object.
 * @ingroup Attribute
 */
template <typename T>
inline void readDataValue(T& serverOrClient, const NodeId& id, DataValue& value) {
    value = readAttribute(serverOrClient, id, UA_ATTRIBUTEID_VALUE, UA_TIMESTAMPSTORETURN_BOTH);
}

/**
 * Read the `Value` attribute of a variable node as a Variant object.
 * @ingroup Attribute
 */
template <typename T>
inline void readValue(T& serverOrClient, const NodeId& id, Variant& value) {
    DataValue dv = readAttribute(serverOrClient, id, UA_ATTRIBUTEID_VALUE);
    value.swap(dv->value);
}

/**
 * Read the `DataType` attribute of a variable (type) node as NodeId.
 * @ingroup Attribute
 */
template <typename T>
inline NodeId readDataType(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<NodeId>(serverOrClient, id, UA_ATTRIBUTEID_DATATYPE);
}

/**
 * Read the `ValueRank` attribute of a variable (type) node.
 *
 * Indicates whether the value attribute of the variable node is an array and how many dimensions
 * the array has.
 * @ingroup Attribute
 */
template <typename T>
inline ValueRank readValueRank(T& serverOrClient, const NodeId& id) {
    const auto index = readAttributeScalar<int32_t>(serverOrClient, id, UA_ATTRIBUTEID_VALUERANK);
    return static_cast<ValueRank>(index);
}

/**
 * Read the `ArrayDimensions` attribute of a variable (type) node.
 *
 * Specifies the maximum supported length of each dimension (0 if unknown).
 * @ingroup Attribute
 */
template <typename T>
inline std::vector<uint32_t> readArrayDimensions(T& serverOrClient, const NodeId& id) {
    const auto dv = readAttribute(serverOrClient, id, UA_ATTRIBUTEID_ARRAYDIMENSIONS);
    if (dv.getValuePtr()->isArray()) {
        return dv.getValuePtr()->template getArrayCopy<uint32_t>();
    }
    return {};
}

/**
 * Read the `AccessLevel` attribute of a variable node.
 *
 * For example `::UA_ACCESSLEVELMASK_READ | ::UA_ACCESSLEVELMASK_WRITE`.
 * The access level attribute is used to indicate how the value of a variable can be accessed
 * (read/write) and if it contains current and/or historic data.
 * @ingroup Attribute
 */
template <typename T>
inline uint8_t readAccessLevel(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<uint8_t>(serverOrClient, id, UA_ATTRIBUTEID_ACCESSLEVEL);
}

/**
 * Read the `UserAccessLevel` attribute a variable node.
 *
 * @copydetails readAccessLevel
 * In constrast to the access level, the user acceess level is taking access rights into account.
 * @ingroup Attribute
 */
template <typename T>
inline uint8_t readUserAccessLevel(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<uint8_t>(serverOrClient, id, UA_ATTRIBUTEID_USERACCESSLEVEL);
}

/**
 * Read the `MinimumSamplingInterval` attribute of a variable node.
 *
 * Specifies (in milliseconds) how fast the server can reasonably sample the value for changes.
 * A minimum sampling interval of 0 indicates that the server is to monitor the item continuously.
 * A minimum sampling interval of -1 means indeterminate.
 * @ingroup Attribute
 */
template <typename T>
inline double readMinimumSamplingInterval(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<double>(serverOrClient, id, UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL);
}

/* ---------------------------------------------------------------------------------------------- */

/**
 * Write the `DisplayName` attribute of a node.
 * @copydetails readDisplayName
 * @ingroup Attribute
 */
template <typename T>
inline void writeDisplayName(T& serverOrClient, const NodeId& id, const LocalizedText& name) {
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_DISPLAYNAME, DataValue::fromScalar(name));
}

/**
 * Write the `Description` attribute of a node.
 * @copydetails readDescription
 * @ingroup Attribute
 */
template <typename T>
inline void writeDescription(T& serverOrClient, const NodeId& id, const LocalizedText& desc) {
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_DESCRIPTION, DataValue::fromScalar(desc));
}

/**
 * Write the `WriteMask` attribute of a node.
 * @copydetails readWriteMask
 * @ingroup Attribute
 */
template <typename T>
inline void writeWriteMask(T& serverOrClient, const NodeId& id, uint32_t mask) {
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_WRITEMASK, DataValue::fromScalar(mask));
}

/**
 * Write the `UserWriteMask` attribute of a node.
 * @copydetails readUserWriteMask
 * @note Cannot be written from the server.
 * @ingroup Attribute
 */
template <typename T>
inline void writeUserWriteMask(T& serverOrClient, const NodeId& id, uint32_t mask) {
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_USERWRITEMASK, DataValue::fromScalar(mask));
}

/**
 * Write the `IsAbstract` attribute of a reference type node.
 * @copydetails readIsAbstract
 * @ingroup Attribute
 */
template <typename T>
inline void writeIsAbstract(T& serverOrClient, const NodeId& id, bool isAbstract) {
    writeAttribute(
        serverOrClient, id, UA_ATTRIBUTEID_ISABSTRACT, DataValue::fromScalar(isAbstract)
    );
}

/**
 * Write the `Symmetric` attribute of a reference type node.
 * @copydetails readSymmetric
 * @ingroup Attribute
 */
template <typename T>
inline void writeSymmetric(T& serverOrClient, const NodeId& id, bool symmetric) {
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_SYMMETRIC, DataValue::fromScalar(symmetric));
}

/**
 * Write the `InverseName` of a reference type node.
 * @copydetails readInverseName
 * @ingroup Attribute
 */
template <typename T>
inline void writeInverseName(T& serverOrClient, const NodeId& id, const LocalizedText& name) {
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_INVERSENAME, DataValue::fromScalar(name));
}

/**
 * Write the `Value` attribute of a variable node as a DataValue object.
 * @copydetails readDataValue
 * @ingroup Attribute
 */
template <typename T>
inline void writeDataValue(T& serverOrClient, const NodeId& id, const DataValue& value) {
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_VALUE, value);
}

/**
 * Write the `Value` attribute of a variable node as a Variant object.
 * @copydetails readValue
 * @ingroup Attribute
 */
template <typename T>
inline void writeValue(T& serverOrClient, const NodeId& id, const Variant& value) {
    UA_DataValue dv{};
    dv.value = *value.handle();  // shallow copy
    dv.hasValue = true;
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_VALUE, asWrapper<DataValue>(dv));
}

/**
 * Write the `DataType` attribute of a variable (type) node.
 * @copydetails readDataType
 * @ingroup Attribute
 */
template <typename T>
inline void writeDataType(T& serverOrClient, const NodeId& id, Type type) {
    const auto typeId = ::opcua::detail::getUaDataType(type)->typeId;
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_DATATYPE, DataValue::fromScalar(typeId));
}

/**
 * Write the `DataType` attribute of a variable (type) node by node id.
 * @copydetails readDataType
 * @ingroup Attribute
 */
template <typename T>
inline void writeDataType(T& serverOrClient, const NodeId& id, const NodeId& typeId) {
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_DATATYPE, DataValue::fromScalar(typeId));
}

/**
 * Write the `ValueRank` attribute of a variable (type) node.
 * @copydetails readValueRank
 * @ingroup Attribute
 */
template <typename T>
inline void writeValueRank(T& serverOrClient, const NodeId& id, ValueRank valueRank) {
    const auto native = static_cast<int32_t>(valueRank);
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_VALUERANK, DataValue::fromScalar(native));
}

/**
 * Write the `ArrayDimensions` attribute of variable (type) node.
 *
 * Should be unspecified if ValueRank is <= 0 (ValueRank::Any, ValueRank::Scalar,
 * ValueRank::ScalarOrOneDimension, ValueRank::OneOrMoreDimensions). The dimension zero is a
 * wildcard and the actual value may have any length in this dimension.
 * @copydetails readArrayDimensions
 * @ingroup Attribute
 */
template <typename T>
inline void writeArrayDimensions(
    T& serverOrClient, const NodeId& id, const std::vector<uint32_t>& dimensions
) {
    writeAttribute(
        serverOrClient, id, UA_ATTRIBUTEID_ARRAYDIMENSIONS, DataValue::fromArray(dimensions)
    );
}

/**
 * Write the `AccessLevel` attribute of variable node.
 * @copydetails readAccessLevel
 * @ingroup Attribute
 */
template <typename T>
inline void writeAccessLevel(T& serverOrClient, const NodeId& id, uint8_t mask) {
    const auto native = static_cast<UA_Byte>(mask);
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_ACCESSLEVEL, DataValue::fromScalar(native));
}

/**
 * Write the `UserAccessLevel` attribute of variable node.
 * @copydetails readUserAccessLevel
 * @note Cannot be written from the server.
 * @ingroup Attribute
 */
template <typename T>
inline void writeUserAccessLevel(T& serverOrClient, const NodeId& id, uint8_t mask) {
    const auto native = static_cast<UA_Byte>(mask);
    writeAttribute(
        serverOrClient, id, UA_ATTRIBUTEID_USERACCESSLEVEL, DataValue::fromScalar(native)
    );
}

/**
 * Write the `MinimumSamplingInterval` attribute of a variable node.
 * @copydetails readMinimumSamplingInterval
 * @ingroup Attribute
 */
template <typename T>
inline void writeMinimumSamplingInterval(T& serverOrClient, const NodeId& id, double milliseconds) {
    writeAttribute(
        serverOrClient,
        id,
        UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL,
        DataValue::fromScalar(milliseconds)
    );
}

}  // namespace opcua::services
