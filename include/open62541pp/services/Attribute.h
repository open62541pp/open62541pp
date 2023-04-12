#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "open62541pp/Common.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

// forward declarations
namespace opcua {
class Server;
}  // namespace opcua

namespace opcua::services {

/**
 * @defgroup Attribute Attribute
 * Read and write node attributes.
 * @ingroup Services
 */

/**
 * Get node class.
 * @ingroup Attribute
 */
NodeClass readNodeClass(Server& server, const NodeId& id);

/**
 * Get browse name.
 * @ingroup Attribute
 */
std::string readBrowseName(Server& server, const NodeId& id);

/**
 * Get localized display name.
 * @ingroup Attribute
 */
LocalizedText readDisplayName(Server& server, const NodeId& id);

/**
 * Get localized description.
 * @ingroup Attribute
 */
LocalizedText readDescription(Server& server, const NodeId& id);

/**
 * Get write mask, for example `::UA_WRITEMASK_ACCESSLEVEL | ::UA_WRITEMASK_DESCRIPTION`.
 * @ingroup Attribute
 */
uint32_t readWriteMask(Server& server, const NodeId& id);

/**
 * Get data type of variable (type) node as NodeId.
 * @ingroup Attribute
 */
NodeId readDataType(Server& server, const NodeId& id);

/**
 * Get value rank of variable (type) node.
 * @ingroup Attribute
 */
ValueRank readValueRank(Server& server, const NodeId& id);

/**
 * Get array dimensions of variable (type) node.
 * @ingroup Attribute
 */
std::vector<uint32_t> readArrayDimensions(Server& server, const NodeId& id);

/**
 * Get access level mask of variable node, for example `::UA_ACCESSLEVELMASK_READ`.
 * @ingroup Attribute
 */
uint8_t readAccessLevel(Server& server, const NodeId& id);

/**
 * Read value from variable node as DataValue object.
 * @ingroup Attribute
 */
void readDataValue(Server& server, const NodeId& id, DataValue& value);

/**
 * Read value from variable node as Variant object.
 * @ingroup Attribute
 */
void readValue(Server& server, const NodeId& id, Variant& value);

/**
 * Set localized display name.
 * @ingroup Attribute
 */
void writeDisplayName(Server& server, const NodeId& id, const LocalizedText& name);

/**
 * Set localized description.
 * @ingroup Attribute
 */
void writeDescription(Server& server, const NodeId& id, const LocalizedText& name);

/**
 * Set write mask, for example `::UA_WRITEMASK_ACCESSLEVEL | ::UA_WRITEMASK_DESCRIPTION`.
 * @ingroup Attribute
 */
void writeWriteMask(Server& server, const NodeId& id, uint32_t mask);

/**
 * Set data type of variable (type) node.
 * @ingroup Attribute
 */
void writeDataType(Server& server, const NodeId& id, Type type);

/**
 * Set data type of variable (type) node by node id.
 * @ingroup Attribute
 */
void writeDataType(Server& server, const NodeId& id, const NodeId& typeId);

/**
 * Set value rank of variable (type) node.
 * @ingroup Attribute
 */
void writeValueRank(Server& server, const NodeId& id, ValueRank valueRank);

/**
 * Set array dimensions of variable (type) node.
 *
 * Should be unspecified if ValueRank is <= 0 (ValueRank::Any, ValueRank::Scalar,
 * ValueRank::ScalarOrOneDimension, ValueRank::OneOrMoreDimensions). The dimension zero is a
 * wildcard and the actual value may have any length in this dimension.
 * @ingroup Attribute
 */
void writeArrayDimensions(
    Server& server, const NodeId& id, const std::vector<uint32_t>& dimensions
);

/**
 * Set access level mask of variable node,
 * for example `::UA_ACCESSLEVELMASK_READ | ::UA_ACCESSLEVELMASK_WRITE`.
 * @ingroup Attribute
 */
void writeAccessLevel(Server& server, const NodeId& id, uint8_t mask);

/**
 * Set modelling rule.
 * @ingroup Attribute
 */
void writeModellingRule(Server& server, const NodeId& id, ModellingRule rule);

/**
 * Write DataValue to variable node.
 * @note open62541 version >=1.1 required
 * @ingroup Attribute
 */
void writeDataValue(Server& server, const NodeId& id, const DataValue& value);

/**
 * Write Variant to variable node.
 * @ingroup Attribute
 */
void writeValue(Server& server, const NodeId& id, const Variant& value);

}  // namespace opcua::services
