#include "open62541pp/Node.h"

#include <cassert>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Helper.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/services/Attribute.h"
#include "open62541pp/services/NodeManagement.h"

#include "open62541_impl.h"

namespace opcua {

Node::Node(const Server& server, const NodeId& id)  // NOLINT
    : server_(server),
      nodeId_(id) {
    // check if node exists
    {
        NodeId outputNode(UA_NODEID_NULL);
        const auto status = UA_Server_readNodeId(server_.handle(), nodeId_, outputNode.handle());
        detail::throwOnBadStatus(status);
    }
}

Server& Node::getServer() noexcept {
    return server_;
}

const Server& Node::getServer() const noexcept {
    return server_;
}

const NodeId& Node::getNodeId() const noexcept {
    return nodeId_;
}

Node Node::addFolder(const NodeId& id, std::string_view browseName, ReferenceType referenceType) {
    services::addFolder(server_, nodeId_, id, browseName, referenceType);
    return {server_, id};
}

Node Node::addObject(
    const NodeId& id,
    std::string_view browseName,
    const NodeId& objectType,
    ReferenceType referenceType
) {
    services::addObject(server_, nodeId_, id, browseName, objectType, referenceType);
    return {server_, id};
}

Node Node::addVariable(
    const NodeId& id,
    std::string_view browseName,
    const NodeId& variableType,
    ReferenceType referenceType
) {
    services::addVariable(server_, nodeId_, id, browseName, variableType, referenceType);
    return {server_, id};
}

Node Node::addProperty(const NodeId& id, std::string_view browseName) {
    services::addProperty(server_, nodeId_, id, browseName);
    return {server_, id};
}

Node Node::addObjectType(
    const NodeId& id, std::string_view browseName, ReferenceType referenceType
) {
    services::addObjectType(server_, nodeId_, id, browseName, referenceType);
    return {server_, id};
}

Node Node::addVariableType(
    const NodeId& id,
    std::string_view browseName,
    const NodeId& variableType,
    ReferenceType referenceType
) {
    services::addVariableType(server_, nodeId_, id, browseName, variableType, referenceType);
    return {server_, id};
}

void Node::addReference(const NodeId& targetId, ReferenceType referenceType, bool forward) {
    services::addReference(server_, nodeId_, targetId, referenceType, forward);
}

void Node::deleteNode(bool deleteReferences) {
    services::deleteNode(server_, nodeId_, deleteReferences);
}

Node Node::getChild(const std::vector<QualifiedName>& path) {
    const std::vector<UA_QualifiedName> pathNative(path.begin(), path.end());
    const TypeWrapper<UA_BrowsePathResult, UA_TYPES_BROWSEPATHRESULT> result(
        UA_Server_browseSimplifiedBrowsePath(
            server_.handle(),
            nodeId_,  // origin
            pathNative.size(),  // browse path size
            pathNative.data()  // browse path
        )
    );
    detail::throwOnBadStatus(result->statusCode);
    assert(result->targets != nullptr && result->targetsSize >= 1);  // NOLINT
    const auto id = ExpandedNodeId(result->targets[0].targetId);  // NOLINT
    if (!id.isLocal()) {
        throw BadStatus(UA_STATUSCODE_BADNOMATCH);
    }
    return {server_, id.getNodeId()};
}

NodeClass Node::readNodeClass() {
    return services::readNodeClass(server_, nodeId_);
}

std::string Node::readBrowseName() {
    return services::readBrowseName(server_, nodeId_);
}

LocalizedText Node::readDisplayName() {
    return services::readDisplayName(server_, nodeId_);
}

LocalizedText Node::readDescription() {
    return services::readDescription(server_, nodeId_);
}

uint32_t Node::readWriteMask() {
    return services::readWriteMask(server_, nodeId_);
}

NodeId Node::readDataType() {
    return services::readDataType(server_, nodeId_);
}

ValueRank Node::readValueRank() {
    return services::readValueRank(server_, nodeId_);
}

std::vector<uint32_t> Node::readArrayDimensions() {
    return services::readArrayDimensions(server_, nodeId_);
}

uint8_t Node::readAccessLevel() {
    return services::readAccessLevel(server_, nodeId_);
}

void Node::readDataValue(DataValue& value) {
    services::readDataValue(server_, nodeId_, value);
}

void Node::readValue(Variant& value) {
    services::readValue(server_, nodeId_, value);
}

void Node::writeDisplayName(std::string_view name, std::string_view locale) {
    services::writeDisplayName(server_, nodeId_, name, locale);
}

void Node::writeDescription(std::string_view name, std::string_view locale) {
    services::writeDescription(server_, nodeId_, name, locale);
}

void Node::writeWriteMask(uint32_t mask) {
    services::writeWriteMask(server_, nodeId_, mask);
}

void Node::writeDataType(Type type) {
    services::writeDataType(server_, nodeId_, type);
}

void Node::writeDataType(const NodeId& typeId) {
    services::writeDataType(server_, nodeId_, typeId);
}

void Node::writeValueRank(ValueRank valueRank) {
    services::writeValueRank(server_, nodeId_, valueRank);
}

void Node::writeArrayDimensions(const std::vector<uint32_t>& dimensions) {
    services::writeArrayDimensions(server_, nodeId_, dimensions);
}

void Node::writeAccessLevel(uint8_t mask) {
    services::writeAccessLevel(server_, nodeId_, mask);
}

void Node::writeModellingRule(ModellingRule rule) {
    services::writeModellingRule(server_, nodeId_, rule);
}

void Node::writeDataValue(const DataValue& value) {
    services::writeDataValue(server_, nodeId_, value);
}

void Node::writeValue(const Variant& value) {
    services::writeValue(server_, nodeId_, value);
}

/* ---------------------------------------------------------------------------------------------- */

bool operator==(const Node& left, const Node& right) noexcept {
    return (left.getServer() == right.getServer()) && (left.getNodeId() == right.getNodeId());
}

bool operator!=(const Node& left, const Node& right) noexcept {
    return !(left == right);
}

}  // namespace opcua
