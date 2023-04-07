#include "open62541pp/Node.h"

#include <cassert>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Helper.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/services/NodeManagement.h"

#include "open62541_impl.h"
#include "version.h"

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
    UA_NodeClass nodeClass = UA_NODECLASS_UNSPECIFIED;
    const auto status = UA_Server_readNodeClass(server_.handle(), nodeId_, &nodeClass);
    detail::throwOnBadStatus(status);
    return static_cast<NodeClass>(nodeClass);
}

std::string Node::readBrowseName() {
    QualifiedName name;
    const auto status = UA_Server_readBrowseName(server_.handle(), nodeId_, name.handle());
    detail::throwOnBadStatus(status);
    return std::string{name.getName()};
}

LocalizedText Node::readDisplayName() {
    LocalizedText text;
    const auto status = UA_Server_readDisplayName(server_.handle(), nodeId_, text.handle());
    detail::throwOnBadStatus(status);
    return text;
}

LocalizedText Node::readDescription() {
    LocalizedText text;
    const auto status = UA_Server_readDescription(server_.handle(), nodeId_, text.handle());
    detail::throwOnBadStatus(status);
    return text;
}

uint32_t Node::readWriteMask() {
    uint32_t writeMask = 0;
    const auto status = UA_Server_readWriteMask(server_.handle(), nodeId_, &writeMask);
    detail::throwOnBadStatus(status);
    return writeMask;
}

NodeId Node::readDataType() {
    NodeId nodeId(0, 0);
    const auto status = UA_Server_readDataType(server_.handle(), nodeId_, nodeId.handle());
    detail::throwOnBadStatus(status);
    return nodeId;
}

ValueRank Node::readValueRank() {
    int32_t valueRank = 0;
    const auto status = UA_Server_readValueRank(server_.handle(), nodeId_, &valueRank);
    detail::throwOnBadStatus(status);
    return static_cast<ValueRank>(valueRank);
}

std::vector<uint32_t> Node::readArrayDimensions() {
    Variant variant;
    const auto status = UA_Server_readArrayDimensions(server_.handle(), nodeId_, variant.handle());
    detail::throwOnBadStatus(status);
    if (variant.isArray()) {
        return variant.getArrayCopy<uint32_t>();
    }
    return {};
}

uint8_t Node::readAccessLevel() {
    uint8_t mask = 0;
    const auto status = UA_Server_readAccessLevel(server_.handle(), nodeId_, &mask);
    detail::throwOnBadStatus(status);
    return mask;
}

void Node::readDataValue(DataValue& value) {
    UA_ReadValueId rvi;
    UA_ReadValueId_init(&rvi);
    rvi.nodeId = *nodeId_.handle();
    rvi.attributeId = UA_ATTRIBUTEID_VALUE;
    value = DataValue(UA_Server_read(server_.handle(), &rvi, UA_TIMESTAMPSTORETURN_BOTH));
    if (value->hasStatus) {
        detail::throwOnBadStatus(value->status);
    }
}

void Node::readValue(Variant& var) {
    const auto status = UA_Server_readValue(server_.handle(), nodeId_, var.handle());
    detail::throwOnBadStatus(status);
}

void Node::writeDisplayName(std::string_view name, std::string_view locale) {
    const auto status = UA_Server_writeDisplayName(
        server_.handle(), nodeId_, LocalizedText(name, locale)
    );
    detail::throwOnBadStatus(status);
}

void Node::writeDescription(std::string_view name, std::string_view locale) {
    const auto status = UA_Server_writeDescription(
        server_.handle(), nodeId_, LocalizedText(name, locale)
    );
    detail::throwOnBadStatus(status);
}

void Node::writeWriteMask(uint32_t mask) {
    const auto status = UA_Server_writeWriteMask(server_.handle(), nodeId_, mask);
    detail::throwOnBadStatus(status);
}

void Node::writeDataType(Type type) {
    const auto status = UA_Server_writeDataType(
        server_.handle(), nodeId_, detail::getUaDataType(type)->typeId
    );
    detail::throwOnBadStatus(status);
}

void Node::writeDataType(const NodeId& typeId) {
    const auto status = UA_Server_writeDataType(server_.handle(), nodeId_, typeId);
    detail::throwOnBadStatus(status);
}

void Node::writeValueRank(ValueRank valueRank) {
    const auto status = UA_Server_writeValueRank(
        server_.handle(), nodeId_, static_cast<int32_t>(valueRank)
    );
    detail::throwOnBadStatus(status);
}

void Node::writeArrayDimensions(const std::vector<uint32_t>& dimensions) {
    Variant variant;
    variant.setArrayCopy(dimensions);
    const auto status = UA_Server_writeArrayDimensions(server_.handle(), nodeId_, variant);
    detail::throwOnBadStatus(status);
}

void Node::writeAccessLevel(uint8_t mask) {
    const auto status = UA_Server_writeAccessLevel(
        server_.handle(), nodeId_, static_cast<UA_Byte>(mask)
    );
    detail::throwOnBadStatus(status);
}

void Node::writeModellingRule(ModellingRule rule) {
    const auto status = UA_Server_addReference(
        server_.handle(),  // server
        nodeId_,  // source id
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),  // reference id
        UA_EXPANDEDNODEID_NUMERIC(0, static_cast<UA_UInt32>(rule)),  // target id
        true  // forward
    );
    detail::throwOnBadStatus(status);
}

void Node::writeDataValue(const DataValue& value) {
    // support for types with optional fields introduced in v1.1
#if UAPP_OPEN62541_VER_GE(1, 1)
    const auto status = UA_Server_writeDataValue(server_.handle(), nodeId_, value);
    detail::throwOnBadStatus(status);
#endif
}

void Node::writeValue(const Variant& var) {
    const auto status = UA_Server_writeValue(server_.handle(), nodeId_, var);
    detail::throwOnBadStatus(status);
}

/* ---------------------------------------------------------------------------------------------- */

bool operator==(const Node& left, const Node& right) noexcept {
    return (left.getServer() == right.getServer()) && (left.getNodeId() == right.getNodeId());
}

bool operator!=(const Node& left, const Node& right) noexcept {
    return !(left == right);
}

}  // namespace opcua
