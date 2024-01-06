#include "open62541pp/services/NodeManagement.h"

#include <cassert>
#include <utility>  // move

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/helper.h"
#include "open62541pp/types/Variant.h"

#include "../ServerContext.h"
#include "../open62541_impl.h"
#include "ClientService.h"

namespace opcua::services {

AddNodesResponse addNodes(Client& client, const AddNodesRequest& request) {
    return sendRequest<UA_AddNodesRequest, UA_AddNodesResponse>(
        client, request, MoveResponse{}, UseSync{}
    );
}

AddReferencesResponse addReferences(Client& client, const AddReferencesRequest& request) {
    return sendRequest<UA_AddReferencesRequest, UA_AddReferencesResponse>(
        client, request, MoveResponse{}, UseSync{}
    );
}

DeleteNodesResponse deleteNodes(Client& client, const DeleteNodesRequest& request) {
    return sendRequest<UA_DeleteNodesRequest, UA_DeleteNodesResponse>(
        client, request, MoveResponse{}, UseSync{}
    );
}

DeleteReferencesResponse deleteReferences(Client& client, const DeleteReferencesRequest& request) {
    return sendRequest<UA_DeleteReferencesRequest, UA_DeleteReferencesResponse>(
        client, request, MoveResponse{}, UseSync{}
    );
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
        {id.getNamespaceIndex(), detail::toNativeString(browseName)},
        typeDefinition.handle(),
        static_cast<const UA_NodeAttributes*>(nodeAttributes.getDecodedData()),
        nodeAttributes.getDecodedDataType(),
        nullptr,  // nodeContext
        addedNodeId.handle()
    );
    detail::throwOnBadStatus(status);
    return addedNodeId;
}

template <typename CompletionHandler>
static auto addNodeImpl(
    Client& client,
    NodeClass nodeClass,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ExtensionObject& nodeAttributes,
    const NodeId& typeDefinition,
    const NodeId& referenceType,
    CompletionHandler&& completionHandler
) {
    UA_AddNodesItem item{};
    item.parentNodeId.nodeId = parentId;
    item.referenceTypeId = referenceType;
    item.requestedNewNodeId.nodeId = id;
    item.browseName.namespaceIndex = id.getNamespaceIndex();
    item.browseName.name = detail::toNativeString(browseName);
    item.nodeClass = static_cast<UA_NodeClass>(nodeClass);
    item.nodeAttributes = nodeAttributes;
    item.typeDefinition.nodeId = typeDefinition;
    UA_AddNodesRequest request{};
    request.nodesToAddSize = 1;
    request.nodesToAdd = &item;
    return sendRequest<UA_AddNodesRequest, UA_AddNodesResponse>(
        client,
        request,
        [](UA_AddNodesResponse& response) {
            auto& result = getSingleResultFromResponse(response);
            detail::throwOnBadStatus(result.statusCode);
            return NodeId(std::exchange(result.addedNodeId, {}));
        },
        std::forward<CompletionHandler>(completionHandler)
    );
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
    return addNodeImpl(
        client,
        nodeClass,
        parentId,
        id,
        browseName,
        nodeAttributes,
        typeDefinition,
        referenceType,
        UseSync{}
    );
}

std::future<NodeId> addNodeAsync(
    Client& client,
    NodeClass nodeClass,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ExtensionObject& nodeAttributes,
    const NodeId& typeDefinition,
    const NodeId& referenceType
) {
    return addNodeImpl(
        client,
        nodeClass,
        parentId,
        id,
        browseName,
        nodeAttributes,
        typeDefinition,
        referenceType,
        UseFuture{}
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
        return detail::invokeCatchStatus([&] {
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
        {id.getNamespaceIndex(), detail::toNativeString(browseName)},
        attributes,
        methodCallback,
        inputArguments.size(),
        asNative(inputArguments.data()),
        outputArguments.size(),
        asNative(outputArguments.data()),
        nodeContext,
        outputNodeId.handle()  // outNewNodeId
    );
    detail::throwOnBadStatus(status);
    return outputNodeId;
}

template <>
NodeId addMethod(
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
    return addNode(
        client,
        NodeClass::Method,
        parentId,
        id,
        browseName,
        ExtensionObject::fromDecoded(const_cast<MethodAttributes&>(attributes)),  // NOLINT
        {},
        referenceType
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
    detail::throwOnBadStatus(status);
}

template <typename CompletionHandler>
static auto addReferenceImpl(
    Client& client,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool forward,
    CompletionHandler&& completionHandler
) {
    UA_AddReferencesItem item{};
    item.sourceNodeId = sourceId;
    item.referenceTypeId = referenceType;
    item.isForward = forward;
    item.targetServerUri = UA_STRING_NULL;
    item.targetNodeId.nodeId = targetId;
    item.targetNodeClass = UA_NODECLASS_UNSPECIFIED;  // necessary?
    UA_AddReferencesRequest request{};
    request.referencesToAddSize = 1;
    request.referencesToAdd = &item;
    return sendRequest<UA_AddReferencesRequest, UA_AddReferencesResponse>(
        client,
        request,
        [](UA_AddReferencesResponse& response) {
            detail::throwOnBadStatus(getSingleResultFromResponse(response));
        },
        std::forward<CompletionHandler>(completionHandler)
    );
}

template <>
void addReference<Client>(
    Client& client,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool forward
) {
    return addReferenceImpl(client, sourceId, targetId, referenceType, forward, UseSync{});
}

std::future<void> addReferenceAsync(
    Client& client,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool forward
) {
    return addReferenceImpl(client, sourceId, targetId, referenceType, forward, UseFuture{});
}

template <>
void deleteNode<Server>(Server& server, const NodeId& id, bool deleteReferences) {
    const auto status = UA_Server_deleteNode(server.handle(), id, deleteReferences);
    detail::throwOnBadStatus(status);
}

template <typename CompletionHandler>
static auto deleteNodeImpl(
    Client& client, const NodeId& id, bool deleteReferences, CompletionHandler&& completionHandler
) {
    UA_DeleteNodesItem item{};
    item.nodeId = id;
    item.deleteTargetReferences = deleteReferences;
    UA_DeleteNodesRequest request{};
    request.nodesToDeleteSize = 1;
    request.nodesToDelete = &item;
    return sendRequest<UA_DeleteNodesRequest, UA_DeleteNodesResponse>(
        client,
        request,
        [](UA_DeleteNodesResponse& response) {
            detail::throwOnBadStatus(getSingleResultFromResponse(response));
        },
        std::forward<CompletionHandler>(completionHandler)
    );
}

template <>
void deleteNode<Client>(Client& client, const NodeId& id, bool deleteReferences) {
    return deleteNodeImpl(client, id, deleteReferences, UseSync{});
}

std::future<void> deleteNodeAsync(Client& client, const NodeId& id, bool deleteReferences) {
    return deleteNodeImpl(client, id, deleteReferences, UseFuture{});
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
    detail::throwOnBadStatus(status);
}

template <typename CompletionHandler>
static auto deleteReferenceImpl(
    Client& client,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool isForward,
    bool deleteBidirectional,
    CompletionHandler&& completionHandler
) {
    UA_DeleteReferencesItem item{};
    item.sourceNodeId = sourceId;
    item.referenceTypeId = referenceType;
    item.isForward = isForward;
    item.targetNodeId.nodeId = targetId;
    item.deleteBidirectional = deleteBidirectional;
    UA_DeleteReferencesRequest request{};
    request.referencesToDeleteSize = 1;
    request.referencesToDelete = &item;
    return sendRequest<UA_DeleteReferencesRequest, UA_DeleteReferencesResponse>(
        client,
        request,
        [](UA_DeleteReferencesResponse& response) {
            detail::throwOnBadStatus(getSingleResultFromResponse(response));
        },
        std::forward<CompletionHandler>(completionHandler)
    );
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
    return deleteReferenceImpl(
        client, sourceId, targetId, referenceType, isForward, deleteBidirectional, UseSync{}
    );
}

std::future<void> deleteReferenceAsync(
    Client& client,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool isForward,
    bool deleteBidirectional
) {
    return deleteReferenceImpl(
        client, sourceId, targetId, referenceType, isForward, deleteBidirectional, UseFuture{}
    );
}

}  // namespace opcua::services
