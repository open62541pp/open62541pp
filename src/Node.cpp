#include "open62541pp/Node.h"

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Helper.h"
#include "open62541pp/TypeWrapper.h"

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
    // store node class
    {
        UA_NodeClass nodeClass = UA_NODECLASS_UNSPECIFIED;
        const auto status = UA_Server_readNodeClass(server_.handle(), nodeId_, &nodeClass);
        detail::throwOnBadStatus(status);
        nodeClass_ = static_cast<NodeClass>(nodeClass);
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

NodeClass Node::getNodeClass() const noexcept {
    return nodeClass_;
}

std::string Node::getBrowseName() {
    QualifiedName name;
    const auto status = UA_Server_readBrowseName(server_.handle(), nodeId_, name.handle());
    detail::throwOnBadStatus(status);
    return name.getName();
}

LocalizedText Node::getDisplayName() {
    LocalizedText text;
    const auto status = UA_Server_readDisplayName(server_.handle(), nodeId_, text.handle());
    detail::throwOnBadStatus(status);
    return text;
}

LocalizedText Node::getDescription() {
    LocalizedText text;
    const auto status = UA_Server_readDescription(server_.handle(), nodeId_, text.handle());
    detail::throwOnBadStatus(status);
    return text;
}

uint32_t Node::getWriteMask() {
    uint32_t writeMask = 0;
    const auto status = UA_Server_readWriteMask(server_.handle(), nodeId_, &writeMask);
    detail::throwOnBadStatus(status);
    return writeMask;
}

NodeId Node::getDataType() {
    NodeId nodeId(0, 0);
    const auto status = UA_Server_readDataType(server_.handle(), nodeId_, nodeId.handle());
    detail::throwOnBadStatus(status);
    return nodeId;
}

ValueRank Node::getValueRank() {
    int32_t valueRank = 0;
    const auto status = UA_Server_readValueRank(server_.handle(), nodeId_, &valueRank);
    detail::throwOnBadStatus(status);
    return static_cast<ValueRank>(valueRank);
}

std::vector<uint32_t> Node::getArrayDimensions() {
    Variant variant;
    const auto status = UA_Server_readArrayDimensions(server_.handle(), nodeId_, variant.handle());
    detail::throwOnBadStatus(status);
    if (variant.isArray()) {
        return variant.getArrayCopy<uint32_t>();
    }
    return {};
}

uint8_t Node::getAccessLevel() {
    uint8_t mask = 0;
    const auto status = UA_Server_readAccessLevel(server_.handle(), nodeId_, &mask);
    detail::throwOnBadStatus(status);
    return mask;
}

void Node::setDisplayName(std::string_view name, std::string_view locale) {
    const auto status = UA_Server_writeDisplayName(
        server_.handle(), nodeId_, LocalizedText(name, locale)
    );
    detail::throwOnBadStatus(status);
}

void Node::setDescription(std::string_view name, std::string_view locale) {
    const auto status = UA_Server_writeDescription(
        server_.handle(), nodeId_, LocalizedText(name, locale)
    );
    detail::throwOnBadStatus(status);
}

void Node::setWriteMask(uint32_t mask) {
    const auto status = UA_Server_writeWriteMask(server_.handle(), nodeId_, mask);
    detail::throwOnBadStatus(status);
}

void Node::setDataType(Type type) {
    const auto status = UA_Server_writeDataType(
        server_.handle(), nodeId_, detail::getUaDataType(type)->typeId
    );
    detail::throwOnBadStatus(status);
}

void Node::setDataType(const NodeId& typeId) {
    const auto status = UA_Server_writeDataType(server_.handle(), nodeId_, typeId);
    detail::throwOnBadStatus(status);
}

void Node::setValueRank(ValueRank valueRank) {
    const auto status = UA_Server_writeValueRank(
        server_.handle(), nodeId_, static_cast<int32_t>(valueRank)
    );
    detail::throwOnBadStatus(status);
}

void Node::setArrayDimensions(const std::vector<uint32_t>& dimensions) {
    Variant variant;
    variant.setArrayCopy(dimensions);
    const auto status = UA_Server_writeArrayDimensions(server_.handle(), nodeId_, variant);
    detail::throwOnBadStatus(status);
}

void Node::setAccessLevel(uint8_t mask) {
    const auto status = UA_Server_writeAccessLevel(
        server_.handle(), nodeId_, static_cast<UA_Byte>(mask)
    );
    detail::throwOnBadStatus(status);
}

void Node::setModellingRule(ModellingRule rule) {
    const auto status = UA_Server_addReference(
        server_.handle(),  // server
        nodeId_,  // source id
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),  // reference id
        UA_EXPANDEDNODEID_NUMERIC(0, static_cast<UA_UInt32>(rule)),  // target id
        true  // forward
    );
    detail::throwOnBadStatus(status);
}

Node Node::addFolder(const NodeId& id, std::string_view browseName, ReferenceType referenceType) {
    const auto status = UA_Server_addObjectNode(
        server_.handle(),  // server
        id,  // new requested id
        nodeId_,  // parent id
        detail::getUaNodeId(referenceType),  // reference id
        QualifiedName(id.getNamespaceIndex(), browseName),  // browse name
        UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE),  // type definition
        UA_ObjectAttributes_default,  // object attributes
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
    return {server_, id};
}

Node Node::addObject(
    const NodeId& id,
    std::string_view browseName,
    const NodeId& objectType,
    ReferenceType referenceType
) {
    const auto status = UA_Server_addObjectNode(
        server_.handle(),  // server
        id,  // new requested id
        nodeId_,  // parent id
        detail::getUaNodeId(referenceType),  // reference id
        QualifiedName(id.getNamespaceIndex(), browseName),  // browse name
        objectType,  // type definition
        UA_ObjectAttributes_default,  // object attributes
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
    return {server_, id};
}

Node Node::addVariable(
    const NodeId& id,
    std::string_view browseName,
    const NodeId& variableType,
    ReferenceType referenceType
) {
    const auto status = UA_Server_addVariableNode(
        server_.handle(),  // server
        id,  // new requested id
        nodeId_,  // parent id
        detail::getUaNodeId(referenceType),  // reference id
        QualifiedName(id.getNamespaceIndex(), browseName),  // browse name
        variableType,  // type definition
        UA_VariableAttributes_default,  // variable attributes
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
    return {server_, id};
}

Node Node::addProperty(const NodeId& id, std::string_view browseName) {
    const auto status = UA_Server_addVariableNode(
        server_.handle(),  // server
        id,  // new requested id
        nodeId_,  // parent id
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),  // reference id
        QualifiedName(id.getNamespaceIndex(), browseName),  // browse name
        UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE),  // type definition
        UA_VariableAttributes_default,  // variable attributes
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
    return {server_, id};
}

Node Node::addObjectType(
    const NodeId& id, std::string_view browseName, ReferenceType referenceType
) {
    const auto status = UA_Server_addObjectTypeNode(
        server_.handle(),  // server
        id,  // new requested id
        nodeId_,  // parent id
        detail::getUaNodeId(referenceType),  // reference id
        QualifiedName(id.getNamespaceIndex(), browseName),  // browse name
        UA_ObjectTypeAttributes_default,  // object attributes
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
    return {server_, id};
}

Node Node::addVariableType(
    const NodeId& id,
    std::string_view browseName,
    const NodeId& variableType,
    ReferenceType referenceType
) {
    const auto status = UA_Server_addVariableTypeNode(
        server_.handle(),  // server
        id,  // new requested id
        nodeId_,  // parent id
        detail::getUaNodeId(referenceType),  // reference id
        QualifiedName(id.getNamespaceIndex(), browseName),  // browse name
        variableType,  // type definition
        UA_VariableTypeAttributes_default,  // variable attributes
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
    return {server_, id};
}

void Node::addReference(const NodeId& target, ReferenceType referenceType, bool forward) {
    const auto status = UA_Server_addReference(
        server_.handle(),  // server
        nodeId_,  // source id
        detail::getUaNodeId(referenceType),  // reference id
        ExpandedNodeId(target, {}, 0),  // target id
        forward  // forward
    );
    detail::throwOnBadStatus(status);
}

void Node::writeValue(const Variant& var) {
    const auto status = UA_Server_writeValue(server_.handle(), nodeId_, var);
    detail::throwOnBadStatus(status);
}

void Node::readValue(Variant& var) {
    const auto status = UA_Server_readValue(server_.handle(), nodeId_, var.handle());
    detail::throwOnBadStatus(status);
}

void Node::remove(bool deleteReferences) {
    const auto status = UA_Server_deleteNode(server_.handle(), nodeId_, deleteReferences);
    detail::throwOnBadStatus(status);
}

}  // namespace opcua
