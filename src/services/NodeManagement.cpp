#include "open62541pp/services/NodeManagement.h"

#include <memory>
#include <stdexcept>
#include <utility>  // move

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/Variant.h"

#include "../ServerContext.h"
#include "../open62541_impl.h"

namespace opcua::services {

template <>
void addObject<Server>(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& objectType,
    const NodeId& referenceType
) {
    const auto status = UA_Server_addObjectNode(
        server.handle(),
        id,
        parentId,
        referenceType,
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
    const NodeId& referenceType
) {
    const auto status = UA_Client_addObjectNode(
        client.handle(),
        id,
        parentId,
        referenceType,
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
    const NodeId& referenceType
) {
    const auto status = UA_Server_addVariableNode(
        server.handle(),
        id,
        parentId,
        referenceType,
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
    const NodeId& referenceType
) {
    const auto status = UA_Client_addVariableNode(
        client.handle(),
        id,
        parentId,
        referenceType,
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
    const NodeId& referenceType
) {
    const auto status = UA_Server_addObjectTypeNode(
        server.handle(),
        id,
        parentId,
        referenceType,
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
    const NodeId& referenceType
) {
    const auto status = UA_Client_addObjectTypeNode(
        client.handle(),
        id,
        parentId,
        referenceType,
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
    const NodeId& referenceType
) {
    const auto status = UA_Server_addVariableTypeNode(
        server.handle(),
        id,
        parentId,
        referenceType,
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
    const NodeId& referenceType
) {
    (void)variableType;  // TODO: variableType is currently unused
    const auto status = UA_Client_addVariableTypeNode(
        client.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        UA_VariableTypeAttributes_default,
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

#ifdef UA_ENABLE_METHODCALLS
static UA_StatusCode methodCallback(
    [[maybe_unused]] UA_Server* server,
    [[maybe_unused]] const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    [[maybe_unused]] const UA_NodeId* methodId,
    void* methodContext,
    [[maybe_unused]] const UA_NodeId* objectId,
    [[maybe_unused]] void* objectContext,
    size_t inputSize,
    const UA_Variant* input,
    size_t outputSize,
    UA_Variant* output
) {
    if (methodContext == nullptr) {
        return UA_STATUSCODE_BADUNEXPECTEDERROR;
    }
    const auto* nodeContext = static_cast<ServerContext::NodeContext*>(methodContext);
    const std::vector<Variant> inputVector(input, input + inputSize);  // NOLINT
    std::vector<Variant> outputVector(outputSize);
    try {
        if (nodeContext->methodCallback) {
            nodeContext->methodCallback(inputVector, outputVector);
            for (size_t i = 0; i < outputSize; ++i) {
                outputVector[i].swap(output[i]);  // NOLINT
            }
        }
    } catch (const BadStatus& e) {
        return e.code();
    } catch (const std::exception&) {
        return UA_STATUSCODE_BAD;
    }
    return UA_STATUSCODE_GOOD;
}

template <>
void addMethod(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    MethodCallback callback,
    const std::vector<Argument>& inputArguments,
    const std::vector<Argument>& outputArguments,
    const NodeId& referenceType
) {
    auto nodeContext = std::make_unique<ServerContext::NodeContext>();
    nodeContext->methodCallback = std::move(callback);
    const auto status = UA_Server_addMethodNode(
        server.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        UA_MethodAttributes_default,
        methodCallback,
        inputArguments.size(),
        inputArguments.data()->handle(),
        outputArguments.size(),
        outputArguments.data()->handle(),
        nodeContext.get(),
        nullptr  // outNewNodeId
    );
    detail::throwOnBadStatus(status);
    server.getContext().nodeContexts.insert_or_assign(id, std::move(nodeContext));
}

template <>
void addMethod(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    MethodCallback callback,  // NOLINT
    const std::vector<Argument>& inputArguments,
    const std::vector<Argument>& outputArguments,
    const NodeId& referenceType
) {
    // callback can be added later by server with UA_Server_setMethodNodeCallback
    (void)callback;
    // arguments can not be passed to UA_Client_addMethodNode... why?
    (void)inputArguments;
    (void)outputArguments;
    const auto status = UA_Client_addMethodNode(
        client.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        UA_MethodAttributes_default,
        nullptr  // outNewNodeId
    );
    detail::throwOnBadStatus(status);
}
#endif

template <>
void addReference<Server>(
    Server& server,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool forward
) {
    const auto status = UA_Server_addReference(
        server.handle(),
        sourceId,
        referenceType,
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
    const NodeId& referenceType,
    bool forward
) {
    const auto status = UA_Client_addReference(
        client.handle(),
        sourceId,
        referenceType,
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
