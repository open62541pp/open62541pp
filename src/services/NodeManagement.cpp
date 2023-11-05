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

template <typename T, typename Native>
inline static void assignArray(Span<const T> array, Native*& ptr, size_t& length) noexcept {
    ptr = asNative(const_cast<T*>(array.data()));  // NOLINT, won't be modified
    length = array.size();
}

AddNodesResponse addNodes(Client& client, const AddNodesRequest& request) {
    AddNodesResponse response = UA_Client_Service_addNodes(client.handle(), request);
    detail::throwOnBadStatus(response->responseHeader.serviceResult);
    return response;
}

AddNodesResponse addNodes(Client& client, Span<const AddNodesItem> nodesToAdd) {
    // avoid copy of nodesToAdd
    UA_AddNodesRequest request{};
    assignArray(nodesToAdd, request.nodesToAdd, request.nodesToAddSize);
    return addNodes(client, asWrapper<AddNodesRequest>(request));
}

AddReferencesResponse addReferences(Client& client, const AddReferencesRequest& request) {
    AddReferencesResponse response = UA_Client_Service_addReferences(client.handle(), request);
    detail::throwOnBadStatus(response->responseHeader.serviceResult);
    return response;
}

AddReferencesResponse addReferences(Client& client, Span<const AddReferencesItem> referencesToAdd) {
    // avoid copy of referencesToAdd
    UA_AddReferencesRequest request{};
    assignArray(referencesToAdd, request.referencesToAdd, request.referencesToAddSize);
    return addReferences(client, asWrapper<AddReferencesRequest>(request));
}

DeleteNodesResponse deleteNodes(Client& client, const DeleteNodesRequest& request) {
    DeleteNodesResponse response = UA_Client_Service_deleteNodes(client.handle(), request);
    detail::throwOnBadStatus(response->responseHeader.serviceResult);
    return response;
}

DeleteNodesResponse deleteNodes(Client& client, Span<const DeleteNodesItem> nodesToDelete) {
    // avoid copy of nodesToDelete
    UA_DeleteNodesRequest request{};
    assignArray(nodesToDelete, request.nodesToDelete, request.nodesToDeleteSize);
    return deleteNodes(client, asWrapper<DeleteNodesRequest>(request));
}

DeleteReferencesResponse deleteReferences(Client& client, const DeleteReferencesRequest& request) {
    DeleteReferencesResponse response = UA_Client_Service_deleteReferences(
        client.handle(), request
    );
    detail::throwOnBadStatus(response->responseHeader.serviceResult);
    return response;
}

DeleteReferencesResponse deleteReferences(
    Client& client, Span<const DeleteReferencesItem> referencesToDelete
) {
    // avoid copy of referencesToDelete
    UA_DeleteReferencesRequest request{};
    assignArray(referencesToDelete, request.referencesToDelete, request.referencesToDeleteSize);
    return deleteReferences(client, asWrapper<DeleteReferencesRequest>(request));
}

template <typename TAttributes>
static NodeId addNode(
    Client& client,
    NodeClass nodeClass,
    const NodeId& parentNodeId,
    const NodeId& requestedNewNodeId,
    const QualifiedName& browseName,
    const TAttributes& nodeAttributes,
    const NodeId& typeDefinition,
    const NodeId& referenceTypeId
) {
    UA_AddNodesItem item{};
    item.parentNodeId.nodeId = parentNodeId;
    item.referenceTypeId = referenceTypeId;
    item.requestedNewNodeId.nodeId = requestedNewNodeId;
    item.browseName = browseName;
    item.nodeClass = static_cast<UA_NodeClass>(nodeClass);
    // NOLINTNEXTLINE, won't be modified
    item.nodeAttributes = ExtensionObject::fromDecoded(const_cast<TAttributes&>(nodeAttributes));
    item.typeDefinition.nodeId = typeDefinition;

    auto response = addNodes(client, {asWrapper<AddNodesItem>(&item), 1});
    auto results = response.getResults();
    if (results.size() != 1) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
    detail::throwOnBadStatus(results[0]->statusCode);
    NodeId addedNodeId;
    addedNodeId.swap(results[0]->addedNodeId);
    return addedNodeId;
}

template <>
NodeId addObject<Server>(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectAttributes& attributes,
    const NodeId& objectType,
    const NodeId& referenceType
) {
    NodeId outputNodeId;
    const auto status = UA_Server_addObjectNode(
        server.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        objectType,
        attributes,
        nullptr,  // node context
        outputNodeId.handle()
    );
    detail::throwOnBadStatus(status);
    return outputNodeId;
}

template <>
NodeId addObject<Client>(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectAttributes& attributes,
    const NodeId& objectType,
    const NodeId& referenceType
) {
    return addNode(
        client,
        NodeClass::Object,
        parentId,
        id,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        objectType,
        referenceType
    );
}

template <>
NodeId addVariable<Server>(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableAttributes& attributes,
    const NodeId& variableType,
    const NodeId& referenceType
) {
    NodeId outputNodeId;
    const auto status = UA_Server_addVariableNode(
        server.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        variableType,
        attributes,
        nullptr,  // node context
        outputNodeId.handle()
    );
    detail::throwOnBadStatus(status);
    return outputNodeId;
}

template <>
NodeId addVariable<Client>(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableAttributes& attributes,
    const NodeId& variableType,
    const NodeId& referenceType
) {
    return addNode(
        client,
        NodeClass::Variable,
        parentId,
        id,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        variableType,
        referenceType
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
        QualifiedName(id.getNamespaceIndex(), browseName),
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
    // callback can be added later by server with UA_Server_setMethodNodeCallback
    // arguments can not be passed to UA_Client_addMethodNode... why?
    return addNode(
        client,
        NodeClass::Method,
        parentId,
        id,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        {},
        referenceType
    );
}
#endif

template <>
NodeId addObjectType<Server>(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectTypeAttributes& attributes,
    const NodeId& referenceType
) {
    NodeId outputNodeId;
    const auto status = UA_Server_addObjectTypeNode(
        server.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        nullptr,  // node context
        outputNodeId.handle()
    );
    detail::throwOnBadStatus(status);
    return outputNodeId;
}

template <>
NodeId addObjectType<Client>(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectTypeAttributes& attributes,
    const NodeId& referenceType
) {
    return addNode(
        client,
        NodeClass::ObjectType,
        parentId,
        id,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        {},
        referenceType
    );
}

template <>
NodeId addVariableType<Server>(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableTypeAttributes& attributes,
    const NodeId& variableType,
    const NodeId& referenceType
) {
    NodeId outputNodeId;
    const auto status = UA_Server_addVariableTypeNode(
        server.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        variableType,
        attributes,
        nullptr,  // node context
        outputNodeId.handle()
    );
    detail::throwOnBadStatus(status);
    return outputNodeId;
}

template <>
NodeId addVariableType<Client>(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableTypeAttributes& attributes,
    const NodeId& variableType,
    const NodeId& referenceType
) {
    return addNode(
        client,
        NodeClass::VariableType,
        parentId,
        id,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        variableType,
        referenceType
    );
}

template <>
NodeId addReferenceType<Server>(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ReferenceTypeAttributes& attributes,
    const NodeId& referenceType
) {
    NodeId outputNodeId;
    const auto status = UA_Server_addReferenceTypeNode(
        server.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        nullptr,  // node context
        outputNodeId.handle()
    );
    detail::throwOnBadStatus(status);
    return outputNodeId;
}

template <>
NodeId addReferenceType<Client>(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ReferenceTypeAttributes& attributes,
    const NodeId& referenceType
) {
    return addNode(
        client,
        NodeClass::ReferenceType,
        parentId,
        id,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        {},
        referenceType
    );
}

template <>
NodeId addDataType<Server>(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const DataTypeAttributes& attributes,
    const NodeId& referenceType
) {
    NodeId outputNodeId;
    const auto status = UA_Server_addDataTypeNode(
        server.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        nullptr,  // node context
        outputNodeId.handle()
    );
    detail::throwOnBadStatus(status);
    return outputNodeId;
}

template <>
NodeId addDataType<Client>(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const DataTypeAttributes& attributes,
    const NodeId& referenceType
) {
    return addNode(
        client,
        NodeClass::DataType,
        parentId,
        id,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        {},
        referenceType
    );
}

template <>
NodeId addView<Server>(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ViewAttributes& attributes,
    const NodeId& referenceType
) {
    NodeId outputNodeId;
    const auto status = UA_Server_addViewNode(
        server.handle(),
        id,
        parentId,
        referenceType,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        nullptr,  // node context
        outputNodeId.handle()
    );
    detail::throwOnBadStatus(status);
    return outputNodeId;
}

template <>
NodeId addView<Client>(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ViewAttributes& attributes,
    const NodeId& referenceType
) {
    return addNode(
        client,
        NodeClass::View,
        parentId,
        id,
        QualifiedName(id.getNamespaceIndex(), browseName),
        attributes,
        {},
        referenceType
    );
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
    UA_AddReferencesItem item{};
    item.sourceNodeId = sourceId;
    item.referenceTypeId = referenceType;
    item.isForward = forward;
    item.targetServerUri = UA_STRING_NULL;
    item.targetNodeId.nodeId = targetId;
    item.targetNodeClass = UA_NODECLASS_UNSPECIFIED;  // necessary?

    auto response = addReferences(client, {asWrapper<AddReferencesItem>(&item), 1});
    auto results = response.getResults();
    if (results.size() != 1) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
    detail::throwOnBadStatus(results[0]);
}

template <>
void deleteNode<Server>(Server& server, const NodeId& id, bool deleteReferences) {
    const auto status = UA_Server_deleteNode(server.handle(), id, deleteReferences);
    detail::throwOnBadStatus(status);
}

template <>
void deleteNode<Client>(Client& client, const NodeId& id, bool deleteReferences) {
    UA_DeleteNodesItem item{};
    item.nodeId = id;
    item.deleteTargetReferences = deleteReferences;

    auto response = deleteNodes(client, {asWrapper<DeleteNodesItem>(&item), 1});
    auto results = response.getResults();
    if (results.size() != 1) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
    detail::throwOnBadStatus(results[0]);
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
    UA_DeleteReferencesItem item{};
    item.sourceNodeId = sourceId;
    item.referenceTypeId = referenceType;
    item.isForward = isForward;
    item.targetNodeId.nodeId = targetId;
    item.deleteBidirectional = deleteBidirectional;

    auto response = deleteReferences(client, {asWrapper<DeleteReferencesItem>(&item), 1});
    auto results = response.getResults();
    if (results.size() != 1) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
    detail::throwOnBadStatus(results[0]);
}

}  // namespace opcua::services
