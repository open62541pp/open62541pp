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
 * @ingroup Services
 */

/**
 * Generic server function to read node attributes.
 * @ingroup Attribute
 */
DataValue readAttribute(
    Server& server,
    const NodeId& id,
    UA_AttributeId attributeId,
    UA_TimestampsToReturn timestamps = UA_TIMESTAMPSTORETURN_NEITHER
);

/**
 * Generic client function to read node attributes.
 * @ingroup Attribute
 */
DataValue readAttribute(
    Client& client,
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
 * Generic server function to write node attributes.
 * @ingroup Attribute
 */
void writeAttribute(
    Server& server, const NodeId& id, UA_AttributeId attributeId, const DataValue& value
);

/**
 * Generic server function to write client attributes.
 * @ingroup Attribute
 */
void writeAttribute(
    Client& client, const NodeId& id, UA_AttributeId attributeId, const DataValue& value
);

/* -------------------------------- Specialized inline functions -------------------------------- */

/**
 * Read node id.
 * @ingroup Attribute
 */
template <typename T>
inline NodeId readNodeId(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<NodeId>(serverOrClient, id, UA_ATTRIBUTEID_NODEID);
}

/**
 * Read node class.
 * @ingroup Attribute
 */
template <typename T>
inline NodeClass readNodeClass(T& serverOrClient, const NodeId& id) {
    auto dv = readAttribute(serverOrClient, id, UA_ATTRIBUTEID_NODECLASS);
    // workaround to read enum from variant...
    return *static_cast<NodeClass*>(dv.getValuePtr()->handle()->data);
}

/**
 * Read browse name.
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
 * Read localized display name.
 * @ingroup Attribute
 */
template <typename T>
inline LocalizedText readDisplayName(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<LocalizedText>(serverOrClient, id, UA_ATTRIBUTEID_DISPLAYNAME);
}

/**
 * Read localized description.
 * @ingroup Attribute
 */
template <typename T>
inline LocalizedText readDescription(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<LocalizedText>(serverOrClient, id, UA_ATTRIBUTEID_DESCRIPTION);
}

/**
 * Read write mask, for example `::UA_WRITEMASK_ACCESSLEVEL | ::UA_WRITEMASK_DESCRIPTION`.
 * @ingroup Attribute
 */
template <typename T>
inline uint32_t readWriteMask(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<uint32_t>(serverOrClient, id, UA_ATTRIBUTEID_WRITEMASK);
}

/**
 * Read data type of variable (type) node as NodeId.
 * @ingroup Attribute
 */
template <typename T>
inline NodeId readDataType(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<NodeId>(serverOrClient, id, UA_ATTRIBUTEID_DATATYPE);
}

/**
 * Read value rank of variable (type) node.
 * @ingroup Attribute
 */
template <typename T>
inline ValueRank readValueRank(T& serverOrClient, const NodeId& id) {
    const auto index = readAttributeScalar<int32_t>(serverOrClient, id, UA_ATTRIBUTEID_VALUERANK);
    return static_cast<ValueRank>(index);
}

/**
 * Read array dimensions of variable (type) node.
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
 * Read access level mask of variable node, for example `::UA_ACCESSLEVELMASK_READ`.
 * @ingroup Attribute
 */
template <typename T>
inline uint8_t readAccessLevel(T& serverOrClient, const NodeId& id) {
    return readAttributeScalar<uint8_t>(serverOrClient, id, UA_ATTRIBUTEID_ACCESSLEVEL);
}

/**
 * Read value from variable node as DataValue object.
 * @ingroup Attribute
 */
template <typename T>
inline void readDataValue(T& serverOrClient, const NodeId& id, DataValue& value) {
    value = readAttribute(serverOrClient, id, UA_ATTRIBUTEID_VALUE, UA_TIMESTAMPSTORETURN_BOTH);
}

/**
 * Read value from variable node as Variant object.
 * @ingroup Attribute
 */
template <typename T>
inline void readValue(T& serverOrClient, const NodeId& id, Variant& value) {
    DataValue dv = readAttribute(serverOrClient, id, UA_ATTRIBUTEID_VALUE);
    value.swap(dv->value);
}

/**
 * Write localized display name.
 * @ingroup Attribute
 */
template <typename T>
inline void writeDisplayName(T& serverOrClient, const NodeId& id, const LocalizedText& name) {
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_DISPLAYNAME, DataValue::fromScalar(name));
}

/**
 * Write localized description.
 * @ingroup Attribute
 */
template <typename T>
inline void writeDescription(T& serverOrClient, const NodeId& id, const LocalizedText& name) {
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_DESCRIPTION, DataValue::fromScalar(name));
}

/**
 * Write write mask, for example `::UA_WRITEMASK_ACCESSLEVEL | ::UA_WRITEMASK_DESCRIPTION`.
 * @ingroup Attribute
 */
template <typename T>
inline void writeWriteMask(T& serverOrClient, const NodeId& id, uint32_t mask) {
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_WRITEMASK, DataValue::fromScalar(mask));
}

/**
 * Write data type of variable (type) node.
 * @ingroup Attribute
 */
template <typename T>
inline void writeDataType(T& serverOrClient, const NodeId& id, Type type) {
    const auto typeId = ::opcua::detail::getUaDataType(type)->typeId;
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_DATATYPE, DataValue::fromScalar(typeId));
}

/**
 * Write data type of variable (type) node by node id.
 * @ingroup Attribute
 */
template <typename T>
inline void writeDataType(T& serverOrClient, const NodeId& id, const NodeId& typeId) {
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_DATATYPE, DataValue::fromScalar(typeId));
}

/**
 * Write value rank of variable (type) node.
 * @ingroup Attribute
 */
template <typename T>
inline void writeValueRank(T& serverOrClient, const NodeId& id, ValueRank valueRank) {
    const auto native = static_cast<int32_t>(valueRank);
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_VALUERANK, DataValue::fromScalar(native));
}

/**
 * Write array dimensions of variable (type) node.
 *
 * Should be unspecified if ValueRank is <= 0 (ValueRank::Any, ValueRank::Scalar,
 * ValueRank::ScalarOrOneDimension, ValueRank::OneOrMoreDimensions). The dimension zero is a
 * wildcard and the actual value may have any length in this dimension.
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
 * Write access level mask of variable node,
 * for example `::UA_ACCESSLEVELMASK_READ | ::UA_ACCESSLEVELMASK_WRITE`.
 * @ingroup Attribute
 */
template <typename T>
inline void writeAccessLevel(T& serverOrClient, const NodeId& id, uint8_t mask) {
    const auto native = static_cast<UA_Byte>(mask);
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_ACCESSLEVEL, DataValue::fromScalar(native));
}

/**
 * Write DataValue to variable node.
 * @ingroup Attribute
 */
template <typename T>
inline void writeDataValue(T& serverOrClient, const NodeId& id, const DataValue& value) {
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_VALUE, value);
}

/**
 * Write Variant to variable node.
 * @ingroup Attribute
 */
template <typename T>
inline void writeValue(T& serverOrClient, const NodeId& id, const Variant& value) {
    UA_DataValue dv{};
    dv.value = *value.handle();  // shallow copy
    dv.hasValue = true;
    writeAttribute(serverOrClient, id, UA_ATTRIBUTEID_VALUE, asWrapper<DataValue>(dv));
}

}  // namespace opcua::services
