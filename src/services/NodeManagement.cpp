#include "open62541pp/services/NodeManagement.h"

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Helper.h"
#include "open62541pp/Server.h"

#include "../open62541_impl.h"

namespace opcua::services {

void addObject(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& objectType,
    ReferenceType referenceType
) {
    const auto status = UA_Server_addObjectNode(
        server.handle(),  // server
        id,  // new requested id
        parentId,  // parent id
        detail::getUaNodeId(referenceType),  // reference id
        QualifiedName(id.getNamespaceIndex(), browseName),  // browse name
        objectType,  // type definition
        UA_ObjectAttributes_default,  // object attributes
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

void addVariable(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& variableType,
    ReferenceType referenceType
) {
    const auto status = UA_Server_addVariableNode(
        server.handle(),  // server
        id,  // new requested id
        parentId,  // parent id
        detail::getUaNodeId(referenceType),  // reference id
        QualifiedName(id.getNamespaceIndex(), browseName),  // browse name
        variableType,  // type definition
        UA_VariableAttributes_default,  // variable attributes
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

void addObjectType(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    ReferenceType referenceType
) {
    const auto status = UA_Server_addObjectTypeNode(
        server.handle(),  // server
        id,  // new requested id
        parentId,  // parent id
        detail::getUaNodeId(referenceType),  // reference id
        QualifiedName(id.getNamespaceIndex(), browseName),  // browse name
        UA_ObjectTypeAttributes_default,  // object attributes
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

void addVariableType(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& variableType,
    ReferenceType referenceType
) {
    const auto status = UA_Server_addVariableTypeNode(
        server.handle(),  // server
        id,  // new requested id
        parentId,  // parent id
        detail::getUaNodeId(referenceType),  // reference id
        QualifiedName(id.getNamespaceIndex(), browseName),  // browse name
        variableType,  // type definition
        UA_VariableTypeAttributes_default,  // variable attributes
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

void addReference(
    Server& server,
    const NodeId& sourceId,
    const NodeId& targetId,
    ReferenceType referenceType,
    bool forward
) {
    const auto status = UA_Server_addReference(
        server.handle(),  // server
        sourceId,  // source id
        detail::getUaNodeId(referenceType),  // reference id
        ExpandedNodeId(targetId, {}, 0),  // target id
        forward  // forward
    );
    detail::throwOnBadStatus(status);
}

void deleteNode(Server& server, const NodeId& id, bool deleteReferences) {
    const auto status = UA_Server_deleteNode(server.handle(), id, deleteReferences);
    detail::throwOnBadStatus(status);
}

}  // namespace opcua::services
