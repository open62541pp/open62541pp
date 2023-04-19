#include "open62541pp/services/NodeManagement.h"

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/detail/helper.h"

#include "../open62541_impl.h"

namespace opcua::services {

template <>
void addObject<Server>(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& objectType,
    ReferenceType referenceType
) {
    const auto status = UA_Server_addObjectNode(
        server.handle(),
        id,
        parentId,
        detail::getUaNodeId(referenceType),
        QualifiedName(id.getNamespaceIndex(), browseName),
        objectType,
        UA_ObjectAttributes_default,
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

template <>
void addObject<Client>(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& objectType,
    ReferenceType referenceType
) {
    const auto status = UA_Client_addObjectNode(
        client.handle(),
        id,
        parentId,
        detail::getUaNodeId(referenceType),
        QualifiedName(id.getNamespaceIndex(), browseName),
        objectType,
        UA_ObjectAttributes_default,
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

template <>
void addVariable<Server>(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& variableType,
    ReferenceType referenceType
) {
    const auto status = UA_Server_addVariableNode(
        server.handle(),
        id,
        parentId,
        detail::getUaNodeId(referenceType),
        QualifiedName(id.getNamespaceIndex(), browseName),
        variableType,
        UA_VariableAttributes_default,
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

template <>
void addVariable<Client>(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& variableType,
    ReferenceType referenceType
) {
    const auto status = UA_Client_addVariableNode(
        client.handle(),
        id,
        parentId,
        detail::getUaNodeId(referenceType),
        QualifiedName(id.getNamespaceIndex(), browseName),
        variableType,
        UA_VariableAttributes_default,
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

template <>
void addObjectType<Server>(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    ReferenceType referenceType
) {
    const auto status = UA_Server_addObjectTypeNode(
        server.handle(),
        id,
        parentId,
        detail::getUaNodeId(referenceType),
        QualifiedName(id.getNamespaceIndex(), browseName),
        UA_ObjectTypeAttributes_default,
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

template <>
void addObjectType<Client>(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    ReferenceType referenceType
) {
    const auto status = UA_Client_addObjectTypeNode(
        client.handle(),
        id,
        parentId,
        detail::getUaNodeId(referenceType),
        QualifiedName(id.getNamespaceIndex(), browseName),
        UA_ObjectTypeAttributes_default,
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

template <>
void addVariableType<Server>(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& variableType,
    ReferenceType referenceType
) {
    const auto status = UA_Server_addVariableTypeNode(
        server.handle(),
        id,
        parentId,
        detail::getUaNodeId(referenceType),
        QualifiedName(id.getNamespaceIndex(), browseName),
        variableType,
        UA_VariableTypeAttributes_default,
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

template <>
void addVariableType<Client>(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& variableType,
    ReferenceType referenceType
) {
    (void)variableType;  // TODO: variableType is currently unused
    const auto status = UA_Client_addVariableTypeNode(
        client.handle(),
        id,
        parentId,
        detail::getUaNodeId(referenceType),
        QualifiedName(id.getNamespaceIndex(), browseName),
        UA_VariableTypeAttributes_default,
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

template <>
void addReference<Server>(
    Server& server,
    const NodeId& sourceId,
    const NodeId& targetId,
    ReferenceType referenceType,
    bool forward
) {
    const auto status = UA_Server_addReference(
        server.handle(),
        sourceId,
        detail::getUaNodeId(referenceType),
        ExpandedNodeId(targetId, {}, 0),
        forward  // isForward
    );
    detail::throwOnBadStatus(status);
}

template <>
void addReference<Client>(
    Client& client,
    const NodeId& sourceId,
    const NodeId& targetId,
    ReferenceType referenceType,
    bool forward
) {
    const auto status = UA_Client_addReference(
        client.handle(),
        sourceId,
        detail::getUaNodeId(referenceType),
        forward,  // isForward
        UA_STRING_NULL,  // targetServerUri
        ExpandedNodeId(targetId, {}, 0),
        UA_NODECLASS_UNSPECIFIED  // targetNodeClass, necessary?
    );
    detail::throwOnBadStatus(status);
}

template <>
void deleteNode<Server>(Server& server, const NodeId& id, bool deleteReferences) {
    const auto status = UA_Server_deleteNode(server.handle(), id, deleteReferences);
    detail::throwOnBadStatus(status);
}

template <>
void deleteNode<Client>(Client& client, const NodeId& id, bool deleteReferences) {
    const auto status = UA_Client_deleteNode(client.handle(), id, deleteReferences);
    detail::throwOnBadStatus(status);
}

}  // namespace opcua::services
