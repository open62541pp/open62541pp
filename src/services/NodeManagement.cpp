#include "open62541pp/services/NodeManagement.h"

#include <cassert>
#include <utility>  // move

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"
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
    const ObjectAttributes& attributes,
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
        attributes,
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
    const ObjectAttributes& attributes,
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
        attributes,
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
    const VariableAttributes& attributes,
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
        attributes,
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
    const VariableAttributes& attributes,
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
        attributes,
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
) noexcept {
    assert(methodContext != nullptr);
    const auto* nodeContext = static_cast<ServerContext::NodeContext*>(methodContext);
    const auto& callback = nodeContext->methodCallback;
    if (callback) {
        return detail::invokeCatchStatus([&] {
            callback(
                {asWrapper<Variant>(input), inputSize}, {asWrapper<Variant>(output), outputSize}
            );
        });
    }
    return UA_STATUSCODE_BADINTERNALERROR;
}

template <>
void addMethod(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    MethodCallback callback,
    Span<const Argument> inputArguments,
    Span<const Argument> outputArguments,
    const MethodAttributes& attributes,
    const NodeId& referenceType
) {
    auto* nodeContext = server.getContext().getOrCreateNodeContext(id);
    nodeContext->methodCallback = std::move(callback);
    const auto status = UA_Server_addMethodNode(
        server.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        methodCallback,
        inputArguments.size(),
        asNative(inputArguments.data()),
        outputArguments.size(),
        asNative(outputArguments.data()),
        nodeContext,
        nullptr  // outNewNodeId
    );
    detail::throwOnBadStatus(status);
}

template <>
void addMethod(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    [[maybe_unused]] MethodCallback callback,  // NOLINT
    [[maybe_unused]] Span<const Argument> inputArguments,
    [[maybe_unused]] Span<const Argument> outputArguments,
    const MethodAttributes& attributes,
    const NodeId& referenceType
) {
    // callback can be added later by server with UA_Server_setMethodNodeCallback
    // arguments can not be passed to UA_Client_addMethodNode... why?
    const auto status = UA_Client_addMethodNode(
        client.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        nullptr  // outNewNodeId
    );
    detail::throwOnBadStatus(status);
}
#endif

template <>
void addObjectType<Server>(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectTypeAttributes& attributes,
    const NodeId& referenceType
) {
    const auto status = UA_Server_addObjectTypeNode(
        server.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
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
    const ObjectTypeAttributes& attributes,
    const NodeId& referenceType
) {
    const auto status = UA_Client_addObjectTypeNode(
        client.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
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
    const VariableTypeAttributes& attributes,
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
        attributes,
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
    const VariableTypeAttributes& attributes,
    [[maybe_unused]] const NodeId& variableType,
    const NodeId& referenceType
) {
    const auto status = UA_Client_addVariableTypeNode(
        client.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

template <>
void addReferenceType<Server>(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ReferenceTypeAttributes& attributes,
    const NodeId& referenceType
) {
    const auto status = UA_Server_addReferenceTypeNode(
        server.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

template <>
void addReferenceType<Client>(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ReferenceTypeAttributes& attributes,
    const NodeId& referenceType
) {
    const auto status = UA_Client_addReferenceTypeNode(
        client.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

template <>
void addDataType<Server>(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const DataTypeAttributes& attributes,
    const NodeId& referenceType
) {
    const auto status = UA_Server_addDataTypeNode(
        server.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

template <>
void addDataType<Client>(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const DataTypeAttributes& attributes,
    const NodeId& referenceType
) {
    const auto status = UA_Client_addDataTypeNode(
        client.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

template <>
void addView<Server>(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ViewAttributes& attributes,
    const NodeId& referenceType
) {
    const auto status = UA_Server_addViewNode(
        server.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        nullptr,  // node context
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

template <>
void addView<Client>(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ViewAttributes& attributes,
    const NodeId& referenceType
) {
    const auto status = UA_Client_addViewNode(
        client.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        nullptr  // output new node id
    );
    detail::throwOnBadStatus(status);
}

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

template <>
void deleteReference<Server>(
    Server& server,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool isForward,
    bool deleteBidirectional
) {
    const auto status = UA_Server_deleteReference(
        server.handle(),
        sourceId,
        referenceType,
        isForward,
        ExpandedNodeId(targetId),
        deleteBidirectional
    );
    detail::throwOnBadStatus(status);
}

template <>
void deleteReference<Client>(
    Client& client,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool isForward,
    bool deleteBidirectional
) {
    const auto status = UA_Client_deleteReference(
        client.handle(),
        sourceId,
        referenceType,
        isForward,
        ExpandedNodeId(targetId),
        deleteBidirectional
    );
    detail::throwOnBadStatus(status);
}

}  // namespace opcua::services
