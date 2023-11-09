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
    return sendRequest<AddNodesRequest, AddNodesResponse>(
        client, request, ForwardResponse<AddNodesResponse>{}
    );
}

AddReferencesResponse addReferences(Client& client, const AddReferencesRequest& request) {
    return sendRequest<AddReferencesRequest, AddReferencesResponse>(
        client, request, ForwardResponse<AddReferencesResponse>{}
    );
}

DeleteNodesResponse deleteNodes(Client& client, const DeleteNodesRequest& request) {
    return sendRequest<DeleteNodesRequest, DeleteNodesResponse>(
        client, request, ForwardResponse<DeleteNodesResponse>{}
    );
}

DeleteReferencesResponse deleteReferences(Client& client, const DeleteReferencesRequest& request) {
    return sendRequest<DeleteReferencesRequest, DeleteReferencesResponse>(
        client, request, ForwardResponse<DeleteReferencesResponse>{}
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
        {id.getNamespaceIndex(), detail::toUaString(browseName)},
        typeDefinition.handle(),
        static_cast<const UA_NodeAttributes*>(nodeAttributes.getDecodedData()),
        nodeAttributes.getDecodedDataType(),
        nullptr,  // nodeContext
        addedNodeId.handle()
    );
    detail::throwOnBadStatus(status);
    return addedNodeId;
}

inline static UA_AddNodesItem createAddNodesItem(
    NodeClass nodeClass,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ExtensionObject& nodeAttributes,
    const NodeId& typeDefinition,
    const NodeId& referenceType
) {
    UA_AddNodesItem item{};
    item.parentNodeId.nodeId = parentId;
    item.referenceTypeId = referenceType;
    item.requestedNewNodeId.nodeId = id;
    item.browseName.namespaceIndex = id.getNamespaceIndex();
    item.browseName.name = detail::toUaString(browseName);
    item.nodeClass = static_cast<UA_NodeClass>(nodeClass);
    item.nodeAttributes = nodeAttributes;
    item.typeDefinition.nodeId = typeDefinition;
    return item;
}

static NodeId getAddedNodeId(AddNodesResponse& response) {
    auto& result = getSingleResultFromResponse(response);
    detail::throwOnBadStatus(result.getStatusCode());
    return std::move(result.getAddedNodeId());
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
    UA_AddNodesItem item = createAddNodesItem(
        nodeClass, parentId, id, browseName, nodeAttributes, typeDefinition, referenceType
    );
    UA_AddNodesRequest request{};
    request.nodesToAddSize = 1;
    request.nodesToAdd = &item;
    return sendRequest<AddNodesRequest, AddNodesResponse>(
        client, asWrapper<AddNodesRequest>(request), &getAddedNodeId
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
    UA_AddNodesItem item = createAddNodesItem(
        nodeClass, parentId, id, browseName, nodeAttributes, typeDefinition, referenceType
    );
    UA_AddNodesRequest request{};
    request.nodesToAddSize = 1;
    request.nodesToAdd = &item;
    return sendAsyncRequest<AddNodesRequest, AddNodesResponse>(
        client, asWrapper<AddNodesRequest>(request), &getAddedNodeId
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
        {id.getNamespaceIndex(), detail::toUaString(browseName)},
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

inline static UA_AddReferencesItem createAddReferencesItem(
    const NodeId& sourceId, const NodeId& targetId, const NodeId& referenceType, bool forward
) {
    UA_AddReferencesItem item{};
    item.sourceNodeId = sourceId;
    item.referenceTypeId = referenceType;
    item.isForward = forward;
    item.targetServerUri = UA_STRING_NULL;
    item.targetNodeId.nodeId = targetId;
    item.targetNodeClass = UA_NODECLASS_UNSPECIFIED;  // necessary?
    return item;
}

static void checkAddReferencesResponse(AddReferencesResponse& response) {
    detail::throwOnBadStatus(getSingleResultFromResponse(response));
}

template <>
void addReference<Client>(
    Client& client,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool forward
) {
    UA_AddReferencesItem item = createAddReferencesItem(sourceId, targetId, referenceType, forward);
    UA_AddReferencesRequest request{};
    request.referencesToAddSize = 1;
    request.referencesToAdd = &item;
    return sendRequest<AddReferencesRequest, AddReferencesResponse>(
        client, asWrapper<AddReferencesRequest>(request), &checkAddReferencesResponse
    );
}

std::future<void> addReferenceAsync(
    Client& client,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool forward
) {
    UA_AddReferencesItem item = createAddReferencesItem(sourceId, targetId, referenceType, forward);
    UA_AddReferencesRequest request{};
    request.referencesToAddSize = 1;
    request.referencesToAdd = &item;
    return sendAsyncRequest<AddReferencesRequest, AddReferencesResponse>(
        client, asWrapper<AddReferencesRequest>(request), &checkAddReferencesResponse
    );
}

template <>
void deleteNode<Server>(Server& server, const NodeId& id, bool deleteReferences) {
    const auto status = UA_Server_deleteNode(server.handle(), id, deleteReferences);
    detail::throwOnBadStatus(status);
}

inline static UA_DeleteNodesItem createDeleteNodesItem(const NodeId& id, bool deleteReferences) {
    UA_DeleteNodesItem item{};
    item.nodeId = id;
    item.deleteTargetReferences = deleteReferences;
    return item;
}

static void checkDeleteNodesResponse(DeleteNodesResponse& response) {
    detail::throwOnBadStatus(getSingleResultFromResponse(response));
}

template <>
void deleteNode<Client>(Client& client, const NodeId& id, bool deleteReferences) {
    UA_DeleteNodesItem item = createDeleteNodesItem(id, deleteReferences);
    UA_DeleteNodesRequest request{};
    request.nodesToDeleteSize = 1;
    request.nodesToDelete = &item;
    return sendRequest<DeleteNodesRequest, DeleteNodesResponse>(
        client, asWrapper<DeleteNodesRequest>(request), &checkDeleteNodesResponse
    );
}

std::future<void> deleteNodeAsync(Client& client, const NodeId& id, bool deleteReferences) {
    UA_DeleteNodesItem item = createDeleteNodesItem(id, deleteReferences);
    UA_DeleteNodesRequest request{};
    request.nodesToDeleteSize = 1;
    request.nodesToDelete = &item;
    return sendAsyncRequest<DeleteNodesRequest, DeleteNodesResponse>(
        client, asWrapper<DeleteNodesRequest>(request), &checkDeleteNodesResponse
    );
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

inline static UA_DeleteReferencesItem createDeleteReferencesItem(
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool isForward,
    bool deleteBidirectional
) {
    UA_DeleteReferencesItem item{};
    item.sourceNodeId = sourceId;
    item.referenceTypeId = referenceType;
    item.isForward = isForward;
    item.targetNodeId.nodeId = targetId;
    item.deleteBidirectional = deleteBidirectional;
    return item;
}

static void checkDeleteReferencesResponse(DeleteReferencesResponse& response) {
    detail::throwOnBadStatus(getSingleResultFromResponse(response));
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
    UA_DeleteReferencesItem item = createDeleteReferencesItem(
        sourceId, targetId, referenceType, isForward, deleteBidirectional
    );
    UA_DeleteReferencesRequest request{};
    request.referencesToDeleteSize = 1;
    request.referencesToDelete = &item;
    return sendRequest<DeleteReferencesRequest, DeleteReferencesResponse>(
        client, asWrapper<DeleteReferencesRequest>(request), &checkDeleteReferencesResponse
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
    UA_DeleteReferencesItem item = createDeleteReferencesItem(
        sourceId, targetId, referenceType, isForward, deleteBidirectional
    );
    UA_DeleteReferencesRequest request{};
    request.referencesToDeleteSize = 1;
    request.referencesToDelete = &item;
    return sendAsyncRequest<DeleteReferencesRequest, DeleteReferencesResponse>(
        client, asWrapper<DeleteReferencesRequest>(request), &checkDeleteReferencesResponse
    );
}

}  // namespace opcua::services
