#include "open62541pp/services/NodeManagement.h"

#include <cassert>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/Result.h"  // tryInvoke
#include "open62541pp/detail/helper.h"

#include "../ServerContext.h"
#include "../open62541_impl.h"

namespace opcua::services {

AddNodesResponse addNodes(Client& client, const AddNodesRequest& request) {
    return addNodesAsync(client, request, detail::SyncOperation{});
}

AddReferencesResponse addReferences(Client& client, const AddReferencesRequest& request) {
    return addReferencesAsync(client, request, detail::SyncOperation{});
}

DeleteNodesResponse deleteNodes(Client& client, const DeleteNodesRequest& request) {
    return deleteNodesAsync(client, request, detail::SyncOperation{});
}

DeleteReferencesResponse deleteReferences(Client& client, const DeleteReferencesRequest& request) {
    return deleteReferencesAsync(client, request, detail::SyncOperation{});
}

template <>
NodeId addNode<Server>(
    Server& server,
    NodeClass nodeClass,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ExtensionObject& nodeAttributes,
    const NodeId& typeDefinition,
    const NodeId& referenceType
) {
    NodeId addedNodeId;
    const auto status = __UA_Server_addNode(
        server.handle(),
        static_cast<UA_NodeClass>(nodeClass),
        id.handle(),
        parentId.handle(),
        referenceType.handle(),
        {id.getNamespaceIndex(), opcua::detail::toNativeString(browseName)},
        typeDefinition.handle(),
        static_cast<const UA_NodeAttributes*>(nodeAttributes.getDecodedData()),
        nodeAttributes.getDecodedDataType(),
        nullptr,  // nodeContext
        addedNodeId.handle()
    );
    opcua::detail::throwOnBadStatus(status);
    return addedNodeId;
}

template <>
NodeId addNode<Client>(
    Client& client,
    NodeClass nodeClass,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ExtensionObject& nodeAttributes,
    const NodeId& typeDefinition,
    const NodeId& referenceType
) {
    return addNodeAsync(
        client,
        nodeClass,
        parentId,
        id,
        browseName,
        nodeAttributes,
        typeDefinition,
        referenceType,
        detail::SyncOperation{}
    );
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
        return opcua::detail::tryInvokeGetStatus([&] {
            callback(
                {asWrapper<Variant>(input), inputSize}, {asWrapper<Variant>(output), outputSize}
            );
        });
    }
    return UA_STATUSCODE_BADINTERNALERROR;
}

template <>
NodeId addMethod(
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
    NodeId outputNodeId;
    const auto status = UA_Server_addMethodNode(
        server.handle(),
        id,
        parentId,
        referenceType,
        {id.getNamespaceIndex(), opcua::detail::toNativeString(browseName)},
        attributes,
        methodCallback,
        inputArguments.size(),
        asNative(inputArguments.data()),
        outputArguments.size(),
        asNative(outputArguments.data()),
        nodeContext,
        outputNodeId.handle()  // outNewNodeId
    );
    opcua::detail::throwOnBadStatus(status);
    return outputNodeId;
}

template <>
NodeId addMethod(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    MethodCallback callback,
    Span<const Argument> inputArguments,
    Span<const Argument> outputArguments,
    const MethodAttributes& attributes,
    const NodeId& referenceType
) {
    return addMethodAsync(
        client,
        parentId,
        id,
        browseName,
        std::move(callback),
        inputArguments,
        outputArguments,
        attributes,
        referenceType,
        detail::SyncOperation{}
    );
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
        {targetId, {}, 0},
        forward  // isForward
    );
    opcua::detail::throwOnBadStatus(status);
}

template <>
void addReference<Client>(
    Client& client,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool forward
) {
    return addReferenceAsync(
        client, sourceId, targetId, referenceType, forward, detail::SyncOperation{}
    );
}

template <>
void deleteNode<Server>(Server& server, const NodeId& id, bool deleteReferences) {
    const auto status = UA_Server_deleteNode(server.handle(), id, deleteReferences);
    opcua::detail::throwOnBadStatus(status);
}

template <>
void deleteNode<Client>(Client& client, const NodeId& id, bool deleteReferences) {
    return deleteNodeAsync(client, id, deleteReferences, detail::SyncOperation{});
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
        server.handle(), sourceId, referenceType, isForward, {targetId, {}, 0}, deleteBidirectional
    );
    opcua::detail::throwOnBadStatus(status);
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
    return deleteReferenceAsync(
        client,
        sourceId,
        targetId,
        referenceType,
        isForward,
        deleteBidirectional,
        detail::SyncOperation{}
    );
}

}  // namespace opcua::services
