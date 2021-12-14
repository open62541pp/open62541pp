#include "open62541pp/Node.h"

#include "open62541/types.h"
#include "open62541/server.h"

#include "open62541pp/TypeWrapper.h"
#include "open62541pp/Helper.h"
#include "open62541pp/ErrorHandling.h"

namespace opcua {

Node::Node(const Server& server, const NodeId& id) // NOLINT
    : server_(server), nodeId_(id) {
    // check if node exists
    NodeId outputNode(UA_NODEID_NULL);
    auto status = UA_Server_readNodeId(server_.handle(), *nodeId_.handle(), outputNode.handle());
    checkStatusCodeException(status);
}

NodeClass Node::getNodeClass() {
    UA_NodeClass nodeClass = UA_NODECLASS_UNSPECIFIED;
    auto status = UA_Server_readNodeClass(server_.handle(), *nodeId_.handle(), &nodeClass);
    checkStatusCodeException(status);
    return static_cast<NodeClass>(nodeClass);
}

std::string Node::getBrowseName() {
    QualifiedName name(0, "");
    auto status = UA_Server_readBrowseName(server_.handle(), *nodeId_.handle(), name.handle());
    checkStatusCodeException(status);
    return name.getName();
}

std::string Node::getDisplayName() {
    LocalizedText text("");
    auto status = UA_Server_readDisplayName(server_.handle(), *nodeId_.handle(), text.handle());
    checkStatusCodeException(status);
    return text.getText();
}

std::string Node::getDescription() {
    LocalizedText text("");
    auto status = UA_Server_readDescription(server_.handle(), *nodeId_.handle(), text.handle());
    checkStatusCodeException(status);
    return text.getText();
}

uint32_t Node::getWriteMask() {
    uint32_t writeMask = 0;
    auto status = UA_Server_readWriteMask(server_.handle(), *nodeId_.handle(), &writeMask);
    checkStatusCodeException(status);
    return writeMask;
}

// void Node::setBrowseName(std::string_view name) {
//     auto status = UA_Server_writeBrowseName(server_.handle(), *nodeId_.handle(),
//         *QualifiedName(nodeId_.getNamespaceIndex(), name).handle());
//     checkStatusCodeException(status);
// }

void Node::setDisplayName(std::string_view name, std::string_view locale) {
    auto status = UA_Server_writeDisplayName(server_.handle(), *nodeId_.handle(),
        *LocalizedText(name, locale).handle());
    checkStatusCodeException(status);
}

void Node::setDescription(std::string_view name, std::string_view locale) {
    auto status = UA_Server_writeDescription(server_.handle(), *nodeId_.handle(),
        *LocalizedText(name, locale).handle());
    checkStatusCodeException(status);
}

void Node::setWriteMask(uint32_t mask) {
    auto status = UA_Server_writeWriteMask(server_.handle(), *nodeId_.handle(), mask);
    checkStatusCodeException(status);
}

// void Node::setDataType(Type type) {
//     auto status = UA_Server_writeDataType(server_.handle(), *nodeId_.handle(),
//         UA_TYPES[static_cast<uint16_t>(type)].typeId);
//     checkStatusCodeException(status);
// }

ObjectNode Node::addFolder(const NodeId& id, std::string_view browseName) {
    auto attr = UA_ObjectAttributes_default;

    auto ns     = id.handle()->namespaceIndex;
    auto status = UA_Server_addObjectNode(
        server_.handle(),                           // server
        *id.handle(),                               // new requested id
        *nodeId_.handle(),                          // parent id
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),   // reference id
        *QualifiedName(ns, browseName).handle(),    // browse name
        UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE),  // type definition
        attr,                                       // object attributes
        nullptr,                                    // node context
        nullptr                                     // output new node id
    );
    checkStatusCodeException(status);

    return ObjectNode(server_, id);
}

ObjectNode Node::addObject(const NodeId& id, std::string_view browseName) {
    auto attr = UA_ObjectAttributes_default;

    auto ns     = id.handle()->namespaceIndex;
    auto status = UA_Server_addObjectNode(
        server_.handle(),                              // server
        *id.handle(),                                  // new requested id
        *nodeId_.handle(),                             // parent id
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),      // reference id
        *QualifiedName(ns, browseName).handle(),       // browse name
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE), // type definition
        attr,                                          // object attributes
        nullptr,                                       // node context
        nullptr                                        // output new node id
    );
    checkStatusCodeException(status);

    return ObjectNode(server_, id);
}

VariableNode Node::addVariable(const NodeId& id, std::string_view browseName, Type type) {
    auto attr        = UA_VariableAttributes_default;
    attr.dataType    = UA_TYPES[static_cast<uint16_t>(type)].typeId; // NOLINT
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    auto ns     = id.handle()->namespaceIndex;
    auto status = UA_Server_addVariableNode(
        server_.handle(),                                    // server
        *id.handle(),                                        // new requested id
        *nodeId_.handle(),                                   // parent id
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),            // reference id
        *QualifiedName(ns, browseName).handle(),             // browse name
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), // type definition
        attr,                                                // variable attributes
        nullptr,                                             // node context
        nullptr                                              // output new node id
    );
    checkStatusCodeException(status);

    return VariableNode(server_, id);
}

ObjectTypeNode Node::addObjectType(const NodeId& id, std::string_view browseName) {
    auto attr = UA_ObjectTypeAttributes_default;

    auto ns     = id.handle()->namespaceIndex;
    auto status = UA_Server_addObjectTypeNode(
        server_.handle(),                              // server
        *id.handle(),                                  // new requested id
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE), // parent id
        UA_NODEID_NUMERIC(ns, UA_NS0ID_ORGANIZES),     // reference id
        *QualifiedName(0, browseName).handle(),        // browse name
        attr,                                          // object attributes
        nullptr,                                       // node context
        nullptr                                        // output new node id
    );
    checkStatusCodeException(status);

    return ObjectTypeNode(server_, id);
}

VariableTypeNode Node::addVariableType(const NodeId& id, std::string_view browseName, Type type) {
    auto attr       = UA_VariableTypeAttributes_default;
    attr.dataType   = UA_TYPES[static_cast<uint16_t>(type)].typeId; // NOLINT
    attr.isAbstract = false;

    auto ns     = id.handle()->namespaceIndex;
    auto status = UA_Server_addVariableTypeNode(
        server_.handle(),                                    // server
        *id.handle(),                                        // new requested id
        *nodeId_.handle(),                                   // parent id
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),            // reference id
        *QualifiedName(ns, browseName).handle(),             // browse name
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), // type definition
        attr,                                                // variable attributes
        nullptr,                                             // node context
        nullptr                                              // output new node id
    );
    checkStatusCodeException(status);

    return VariableTypeNode(server_, id);
}

void VariableNode::writeVariantToServer(Variant& var) {
    auto status = UA_Server_writeValue(server_.handle(), *nodeId_.handle(), *var.handle());
    checkStatusCodeException(status);
}

void VariableNode::readVariantFromServer(Variant& var) noexcept {
    UA_Server_readValue(server_.handle(), *nodeId_.handle(), var.handle());
}

void Node::remove() {
    auto status = UA_Server_deleteNode(server_.handle(), *nodeId_.handle(), true); // remove all references
    checkStatusCodeException(status);
}

} // namespace opcua
