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
 * Generic server function to write node attributes.
 * @ingroup Attribute
 */
void writeAttribute(
    Server& server, const NodeId& id, UA_AttributeId attributeId, const DataValue& value
);

/**
 * Read node id.
 * @ingroup Attribute
 */
inline NodeId readNodeId(Server& server, const NodeId& id) {
    const auto dv = readAttribute(server, id, UA_ATTRIBUTEID_NODEID);
    return dv.getValuePtr()->getScalarCopy<NodeId>();
}

/**
 * Read node class.
 * @ingroup Attribute
 */
inline NodeClass readNodeClass(Server& server, const NodeId& id) {
    auto dv = readAttribute(server, id, UA_ATTRIBUTEID_NODECLASS);
    // workaround to read enum from variant...
    auto* nodeClass = static_cast<NodeClass*>(dv.getValuePtr()->handle()->data);
    return static_cast<NodeClass>(*nodeClass);
}

/**
 * Read browse name.
 * @ingroup Attribute
 */
inline std::string readBrowseName(Server& server, const NodeId& id) {
    const auto dv = readAttribute(server, id, UA_ATTRIBUTEID_BROWSENAME);
    return std::string(dv.getValuePtr()->getScalarCopy<QualifiedName>().getName());
}

/**
 * Read localized display name.
 * @ingroup Attribute
 */
inline LocalizedText readDisplayName(Server& server, const NodeId& id) {
    const auto dv = readAttribute(server, id, UA_ATTRIBUTEID_DISPLAYNAME);
    return dv.getValuePtr()->getScalarCopy<LocalizedText>();
}

/**
 * Read localized description.
 * @ingroup Attribute
 */
inline LocalizedText readDescription(Server& server, const NodeId& id) {
    const auto dv = readAttribute(server, id, UA_ATTRIBUTEID_DESCRIPTION);
    return dv.getValuePtr()->getScalarCopy<LocalizedText>();
}

/**
 * Read write mask, for example `::UA_WRITEMASK_ACCESSLEVEL | ::UA_WRITEMASK_DESCRIPTION`.
 * @ingroup Attribute
 */
inline uint32_t readWriteMask(Server& server, const NodeId& id) {
    const auto dv = readAttribute(server, id, UA_ATTRIBUTEID_WRITEMASK);
    return dv.getValuePtr()->getScalarCopy<uint32_t>();
}

/**
 * Read data type of variable (type) node as NodeId.
 * @ingroup Attribute
 */
inline NodeId readDataType(Server& server, const NodeId& id) {
    const auto dv = readAttribute(server, id, UA_ATTRIBUTEID_DATATYPE);
    return dv.getValuePtr()->getScalarCopy<NodeId>();
}

/**
 * Read value rank of variable (type) node.
 * @ingroup Attribute
 */
inline ValueRank readValueRank(Server& server, const NodeId& id) {
    const auto dv = readAttribute(server, id, UA_ATTRIBUTEID_VALUERANK);
    return static_cast<ValueRank>(dv.getValuePtr()->getScalarCopy<int32_t>());
}

/**
 * Read array dimensions of variable (type) node.
 * @ingroup Attribute
 */
inline std::vector<uint32_t> readArrayDimensions(Server& server, const NodeId& id) {
    const auto dv = readAttribute(server, id, UA_ATTRIBUTEID_ARRAYDIMENSIONS);
    if (dv.getValuePtr()->isArray()) {
        return dv.getValuePtr()->getArrayCopy<uint32_t>();
    }
    return {};
}

/**
 * Read access level mask of variable node, for example `::UA_ACCESSLEVELMASK_READ`.
 * @ingroup Attribute
 */
inline uint8_t readAccessLevel(Server& server, const NodeId& id) {
    const auto dv = readAttribute(server, id, UA_ATTRIBUTEID_ACCESSLEVEL);
    return dv.getValuePtr()->getScalarCopy<uint8_t>();
}

/**
 * Read value from variable node as DataValue object.
 * @ingroup Attribute
 */
inline void readDataValue(Server& server, const NodeId& id, DataValue& value) {
    value = readAttribute(server, id, UA_ATTRIBUTEID_VALUE, UA_TIMESTAMPSTORETURN_BOTH);
}

/**
 * Read value from variable node as Variant object.
 * @ingroup Attribute
 */
inline void readValue(Server& server, const NodeId& id, Variant& value) {
    value = readAttribute(server, id, UA_ATTRIBUTEID_VALUE).getValue().value();
}

/**
 * Write localized display name.
 * @ingroup Attribute
 */
inline void writeDisplayName(Server& server, const NodeId& id, const LocalizedText& name) {
    writeAttribute(server, id, UA_ATTRIBUTEID_DISPLAYNAME, DataValue::fromScalar(name));
}

/**
 * Write localized description.
 * @ingroup Attribute
 */
inline void writeDescription(Server& server, const NodeId& id, const LocalizedText& name) {
    writeAttribute(server, id, UA_ATTRIBUTEID_DESCRIPTION, DataValue::fromScalar(name));
}

/**
 * Write write mask, for example `::UA_WRITEMASK_ACCESSLEVEL | ::UA_WRITEMASK_DESCRIPTION`.
 * @ingroup Attribute
 */
inline void writeWriteMask(Server& server, const NodeId& id, uint32_t mask) {
    writeAttribute(server, id, UA_ATTRIBUTEID_WRITEMASK, DataValue::fromScalar(mask));
}

/**
 * Write data type of variable (type) node.
 * @ingroup Attribute
 */
inline void writeDataType(Server& server, const NodeId& id, Type type) {
    const auto typeId = detail::getUaDataType(type)->typeId;
    writeAttribute(server, id, UA_ATTRIBUTEID_DATATYPE, DataValue::fromScalar(typeId));
}

/**
 * Write data type of variable (type) node by node id.
 * @ingroup Attribute
 */
inline void writeDataType(Server& server, const NodeId& id, const NodeId& typeId) {
    writeAttribute(server, id, UA_ATTRIBUTEID_DATATYPE, DataValue::fromScalar(typeId));
}

/**
 * Write value rank of variable (type) node.
 * @ingroup Attribute
 */
inline void writeValueRank(Server& server, const NodeId& id, ValueRank valueRank) {
    const auto native = static_cast<int32_t>(valueRank);
    writeAttribute(server, id, UA_ATTRIBUTEID_VALUERANK, DataValue::fromScalar(native));
}

/**
 * Write array dimensions of variable (type) node.
 *
 * Should be unspecified if ValueRank is <= 0 (ValueRank::Any, ValueRank::Scalar,
 * ValueRank::ScalarOrOneDimension, ValueRank::OneOrMoreDimensions). The dimension zero is a
 * wildcard and the actual value may have any length in this dimension.
 * @ingroup Attribute
 */
inline void writeArrayDimensions(
    Server& server, const NodeId& id, const std::vector<uint32_t>& dimensions
) {
    writeAttribute(server, id, UA_ATTRIBUTEID_ARRAYDIMENSIONS, DataValue::fromArray(dimensions));
}

/**
 * Write access level mask of variable node,
 * for example `::UA_ACCESSLEVELMASK_READ | ::UA_ACCESSLEVELMASK_WRITE`.
 * @ingroup Attribute
 */
inline void writeAccessLevel(Server& server, const NodeId& id, uint8_t mask) {
    const auto native = static_cast<UA_Byte>(mask);
    writeAttribute(server, id, UA_ATTRIBUTEID_ACCESSLEVEL, DataValue::fromScalar(native));
}

/**
 * Write DataValue to variable node.
 * @ingroup Attribute
 */
inline void writeDataValue(Server& server, const NodeId& id, const DataValue& value) {
    writeAttribute(server, id, UA_ATTRIBUTEID_VALUE, value);
}

/**
 * Write Variant to variable node.
 * @ingroup Attribute
 */
inline void writeValue(Server& server, const NodeId& id, const Variant& value) {
    UA_DataValue dv{};
    dv.value = *value.handle();  // shallow copy
    dv.hasValue = true;
    writeAttribute(server, id, UA_ATTRIBUTEID_VALUE, asWrapper<DataValue>(dv));
}

}  // namespace opcua::services
