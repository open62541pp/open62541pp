#include "open62541pp/Node.h"

#include <cassert>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Helper.h"
#include "open62541pp/TypeWrapper.h"

#include "open62541_impl.h"

namespace opcua {

Node::Node(const Server& server, const NodeId& id)  // NOLINT
    : server_(server), nodeId_(id) {
    // check if node exists
    {
        NodeId outputNode(UA_NODEID_NULL);
        const auto status = UA_Server_readNodeId(
            server_.handle(), *nodeId_.handle(), outputNode.handle()
        );
        detail::checkStatusCodeException(status);
    }
    // store node class
    {
        UA_NodeClass nodeClass = UA_NODECLASS_UNSPECIFIED;
        const auto status = UA_Server_readNodeClass(
            server_.handle(), *nodeId_.handle(), &nodeClass
        );
        detail::checkStatusCodeException(status);
        nodeClass_ = static_cast<NodeClass>(nodeClass);
    }
}

NodeClass Node::getNodeClass() {
    return nodeClass_;
}

std::string Node::getBrowseName() {
    QualifiedName name(0, "");
    const auto status = UA_Server_readBrowseName(
        server_.handle(), *nodeId_.handle(), name.handle()
    );
    detail::checkStatusCodeException(status);
    return name.getName();
}

std::string Node::getDisplayName() {
    LocalizedText text("");
    const auto status = UA_Server_readDisplayName(
        server_.handle(), *nodeId_.handle(), text.handle()
    );
    detail::checkStatusCodeException(status);
    return text.getText();
}

std::string Node::getDescription() {
    LocalizedText text("");
    const auto status = UA_Server_readDescription(
        server_.handle(), *nodeId_.handle(), text.handle()
    );
    detail::checkStatusCodeException(status);
    return text.getText();
}

uint32_t Node::getWriteMask() {
    uint32_t writeMask = 0;
    const auto status = UA_Server_readWriteMask(server_.handle(), *nodeId_.handle(), &writeMask);
    detail::checkStatusCodeException(status);
    return writeMask;
}

NodeId Node::getDataType() {
    NodeId nodeId(0, 0);
    const auto status = UA_Server_readDataType(
        server_.handle(), *nodeId_.handle(), nodeId.handle()
    );
    detail::checkStatusCodeException(status);
    return nodeId;
}

ValueRank Node::getValueRank() {
    int32_t valueRank = 0;
    const auto status = UA_Server_readValueRank(server_.handle(), *nodeId_.handle(), &valueRank);
    detail::checkStatusCodeException(status);
    return static_cast<ValueRank>(valueRank);
}

std::vector<uint32_t> Node::getArrayDimensions() {
    Variant variant;
    const auto status = UA_Server_readArrayDimensions(
        server_.handle(), *nodeId_.handle(), variant.handle()
    );
    detail::checkStatusCodeException(status);
    if (variant.isArray()) {
        return variant.readArray<uint32_t>();
    }
    return {};
}

uint8_t Node::getAccessLevel() {
    uint8_t mask = 0;
    const auto status = UA_Server_readAccessLevel(server_.handle(), *nodeId_.handle(), &mask);
    detail::checkStatusCodeException(status);
    return mask;
}

void Node::setDisplayName(std::string_view name, std::string_view locale) {
    const auto status = UA_Server_writeDisplayName(
        server_.handle(), *nodeId_.handle(), *LocalizedText(name, locale).handle()
    );
    detail::checkStatusCodeException(status);
}

void Node::setDescription(std::string_view name, std::string_view locale) {
    const auto status = UA_Server_writeDescription(
        server_.handle(), *nodeId_.handle(), *LocalizedText(name, locale).handle()
    );
    detail::checkStatusCodeException(status);
}

void Node::setWriteMask(uint32_t mask) {
    const auto status = UA_Server_writeWriteMask(server_.handle(), *nodeId_.handle(), mask);
    detail::checkStatusCodeException(status);
}

void Node::setDataType(Type type) {
    const auto status = UA_Server_writeDataType(
        server_.handle(), *nodeId_.handle(), getUaDataType(type)->typeId
    );
    detail::checkStatusCodeException(status);
}

void Node::setDataType(const NodeId& typeId) {
    const auto status = UA_Server_writeDataType(
        server_.handle(), *nodeId_.handle(), *typeId.handle()
    );
    detail::checkStatusCodeException(status);
}

void Node::setValueRank(ValueRank valueRank) {
    const auto status = UA_Server_writeValueRank(
        server_.handle(), *nodeId_.handle(), static_cast<int32_t>(valueRank)
    );
    detail::checkStatusCodeException(status);
}

void Node::setArrayDimensions(const std::vector<uint32_t>& dimensions) {
    Variant variant;
    variant.setArray(dimensions);
    const auto status = UA_Server_writeArrayDimensions(
        server_.handle(), *nodeId_.handle(), *variant.handle()
    );
    detail::checkStatusCodeException(status);
}

void Node::setAccessLevel(uint8_t mask) {
    const auto status = UA_Server_writeAccessLevel(
        server_.handle(), *nodeId_.handle(), static_cast<UA_Byte>(mask)
    );
    detail::checkStatusCodeException(status);
}

Node Node::addFolder(const NodeId& id, std::string_view browseName) {
    auto attr = UA_ObjectAttributes_default;

    const auto ns = id.handle()->namespaceIndex;
    const auto status = UA_Server_addObjectNode(
        server_.handle(),  // server
        *id.handle(),  // new requested id
        *nodeId_.handle(),  // parent id
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),  // reference id
        *QualifiedName(ns, browseName).handle(),  // browse name
        UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE),  // type definition
        attr,  // object attributes
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::checkStatusCodeException(status);
    return {server_, id};
}

Node Node::addObject(const NodeId& id, std::string_view browseName) {
    auto attr = UA_ObjectAttributes_default;

    const auto ns = id.handle()->namespaceIndex;
    const auto status = UA_Server_addObjectNode(
        server_.handle(),  // server
        *id.handle(),  // new requested id
        *nodeId_.handle(),  // parent id
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),  // reference id
        *QualifiedName(ns, browseName).handle(),  // browse name
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),  // type definition
        attr,  // object attributes
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::checkStatusCodeException(status);
    return {server_, id};
}

Node Node::addVariable(const NodeId& id, std::string_view browseName, Type type) {
    auto attr = UA_VariableAttributes_default;
    attr.dataType = getUaDataType(type)->typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ;

    const auto ns = id.handle()->namespaceIndex;
    const auto status = UA_Server_addVariableNode(
        server_.handle(),  // server
        *id.handle(),  // new requested id
        *nodeId_.handle(),  // parent id
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),  // reference id
        *QualifiedName(ns, browseName).handle(),  // browse name
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),  // type definition
        attr,  // variable attributes
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::checkStatusCodeException(status);
    return {server_, id};
}

Node Node::addProperty(const NodeId& id, std::string_view browseName, Type type) {
    auto attr = UA_VariableAttributes_default;
    attr.dataType = getUaDataType(type)->typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    const auto ns = id.handle()->namespaceIndex;
    const auto status = UA_Server_addVariableNode(
        server_.handle(),  // server
        *id.handle(),  // new requested id
        *nodeId_.handle(),  // parent id
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),  // reference id
        *QualifiedName(ns, browseName).handle(),  // browse name
        UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE),  // type definition
        attr,  // variable attributes
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::checkStatusCodeException(status);
    return {server_, id};
}

Node Node::addObjectType(const NodeId& id, std::string_view browseName) {
    auto attr = UA_ObjectTypeAttributes_default;

    const auto ns = id.handle()->namespaceIndex;
    const auto status = UA_Server_addObjectTypeNode(
        server_.handle(),  // server
        *id.handle(),  // new requested id
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),  // parent id
        UA_NODEID_NUMERIC(ns, UA_NS0ID_ORGANIZES),  // reference id
        *QualifiedName(0, browseName).handle(),  // browse name
        attr,  // object attributes
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::checkStatusCodeException(status);
    return {server_, id};
}

Node Node::addVariableType(const NodeId& id, std::string_view browseName, Type type) {
    auto attr = UA_VariableTypeAttributes_default;
    attr.dataType = getUaDataType(type)->typeId;
    attr.isAbstract = false;

    const auto ns = id.handle()->namespaceIndex;
    const auto status = UA_Server_addVariableTypeNode(
        server_.handle(),  // server
        *id.handle(),  // new requested id
        *nodeId_.handle(),  // parent id
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),  // reference id
        *QualifiedName(ns, browseName).handle(),  // browse name
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),  // type definition
        attr,  // variable attributes
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::checkStatusCodeException(status);
    return {server_, id};
}

void Node::remove() {
    const auto status = UA_Server_deleteNode(
        server_.handle(),
        *nodeId_.handle(),
        true  // remove all references
    );
    detail::checkStatusCodeException(status);
}

void Node::writeVariantToServer(Variant& var) {
    const auto status = UA_Server_writeValue(server_.handle(), *nodeId_.handle(), *var.handle());
    detail::checkStatusCodeException(status);
}

void Node::readVariantFromServer(Variant& var) noexcept {
    UA_Server_readValue(server_.handle(), *nodeId_.handle(), var.handle());
}

}  // namespace opcua
