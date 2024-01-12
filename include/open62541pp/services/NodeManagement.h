#pragma once

#include <cstdint>
#include <functional>
#include <string_view>
#include <utility>  // exchange, forward

#include "open62541pp/Client.h"
#include "open62541pp/Common.h"  // ModellingRule
#include "open62541pp/Config.h"
#include "open62541pp/NodeIds.h"  // *TypeId
#include "open62541pp/Span.h"
#include "open62541pp/async.h"
#include "open62541pp/detail/ClientService.h"
#include "open62541pp/detail/RequestHandling.h"
#include "open62541pp/detail/ResponseHandling.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

namespace opcua::services {

/**
 * @defgroup NodeManagement NodeManagement service set
 * Add/delete nodes and references.
 *
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7
 * @ingroup Services
 * @{
 */

/**
 * Add one or more nodes (client only).
 */
AddNodesResponse addNodes(Client& client, const AddNodesRequest& request);

/**
 * Asynchronously add one or more nodes (client only).
 * @param token Completion handler with the signature `void(StatusCode, AddNodesResponse&)`.
 */
template <typename CompletionToken = DefaultCompletionToken>
auto addNodesAsync(
    Client& client,
    const AddNodesRequest& request,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<UA_AddNodesRequest, UA_AddNodesResponse>(
        client, request, detail::MoveResponse{}, std::forward<CompletionToken>(token)
    );
}

/**
 * Add one or more references (client only).
 */
AddReferencesResponse addReferences(Client& client, const AddReferencesRequest& request);

/**
 * Asynchronously add one or more references (client only).
 */
template <typename CompletionToken = DefaultCompletionToken>
auto addReferencesAsync(
    Client& client,
    const AddReferencesRequest& request,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<UA_AddReferencesRequest, UA_AddReferencesResponse>(
        client, request, detail::MoveResponse{}, std::forward<CompletionToken>(token)
    );
}

/**
 * Delete one or more nodes (client only).
 */
DeleteNodesResponse deleteNodes(Client& client, const DeleteNodesRequest& request);

/**
 * Asynchronously delete one or more nodes (client only).
 */
template <typename CompletionToken = DefaultCompletionToken>
auto deleteNodesAsync(
    Client& client,
    const DeleteNodesRequest& request,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<UA_DeleteNodesRequest, UA_DeleteNodesResponse>(
        client, request, detail::MoveResponse{}, std::forward<CompletionToken>(token)
    );
}

/**
 * Delete one or more references (client only).
 */
DeleteReferencesResponse deleteReferences(Client& client, const DeleteReferencesRequest& request);

/**
 * Asynchronously delete one or more references (client only).
 */
template <typename CompletionToken = DefaultCompletionToken>
auto deleteReferencesAsync(
    Client& client,
    const DeleteReferencesRequest& request,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<UA_DeleteReferencesRequest, UA_DeleteReferencesResponse>(
        client, request, detail::MoveResponse{}, std::forward<CompletionToken>(token)
    );
}

/**
 * Add a node.
 * @exception BadStatus
 */
template <typename T>
NodeId addNode(
    T& serverOrClient,
    NodeClass nodeClass,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ExtensionObject& nodeAttributes,
    const NodeId& typeDefinition,
    const NodeId& referenceType
);

/**
 * Asynchronously add a node.
 * @copydetails addNode
 */
template <typename CompletionToken = DefaultCompletionToken>
auto addNodeAsync(
    Client& client,
    NodeClass nodeClass,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ExtensionObject& nodeAttributes,
    const NodeId& typeDefinition,
    const NodeId& referenceType,
    CompletionToken&& token = DefaultCompletionToken()
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
    return detail::sendRequest<UA_AddNodesRequest, UA_AddNodesResponse>(
        client,
        request,
        [](UA_AddNodesResponse& response) {
            auto& result = detail::getSingleResult(response);
            detail::throwOnBadStatus(result.statusCode);
            return NodeId(std::exchange(result.addedNodeId, {}));
        },
        std::forward<CompletionToken>(token)
    );
}

/* ------------------------------- Specialized (inline) functions ------------------------------- */

/**
 * Add object.
 * @exception BadStatus
 */
template <typename T>
inline NodeId addObject(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectAttributes& attributes = {},
    const NodeId& objectType = ObjectTypeId::BaseObjectType,
    const NodeId& referenceType = ReferenceTypeId::HasComponent
) {
    return addNode(
        serverOrClient,
        NodeClass::Object,
        parentId,
        id,
        browseName,
        detail::wrapNodeAttributes(attributes),
        objectType,
        referenceType
    );
}

/**
 * Asynchronously add object.
 * @copydetails addObject
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto addObjectAsync(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectAttributes& attributes = {},
    const NodeId& objectType = ObjectTypeId::BaseObjectType,
    const NodeId& referenceType = ReferenceTypeId::HasComponent,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addNodeAsync(
        client,
        NodeClass::Object,
        parentId,
        id,
        browseName,
        detail::wrapNodeAttributes(attributes),
        objectType,
        referenceType,
        std::forward<CompletionToken>(token)
    );
}

/**
 * Add folder.
 * @exception BadStatus
 */
template <typename T>
inline NodeId addFolder(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasComponent
) {
    return addObject(
        serverOrClient,
        parentId,
        id,
        browseName,
        attributes,
        ObjectTypeId::FolderType,
        referenceType
    );
}

/**
 * Asynchronously add folder.
 * @copydetails addFolder
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto addFolderAsync(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasComponent,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addObjectAsync(
        client,
        parentId,
        id,
        browseName,
        attributes,
        ObjectTypeId::FolderType,
        referenceType,
        std::forward<CompletionToken>(token)
    );
}

/**
 * Add variable.
 * @exception BadStatus
 */
template <typename T>
inline NodeId addVariable(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableAttributes& attributes = {},
    const NodeId& variableType = VariableTypeId::BaseDataVariableType,
    const NodeId& referenceType = ReferenceTypeId::HasComponent
) {
    return addNode(
        serverOrClient,
        NodeClass::Variable,
        parentId,
        id,
        browseName,
        detail::wrapNodeAttributes(attributes),
        variableType,
        referenceType
    );
}

/**
 * Asynchronously add variable.
 * @copydetails addVariable
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto addVariableAsync(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableAttributes& attributes = {},
    const NodeId& variableType = VariableTypeId::BaseDataVariableType,
    const NodeId& referenceType = ReferenceTypeId::HasComponent,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addNodeAsync(
        client,
        NodeClass::Variable,
        parentId,
        id,
        browseName,
        detail::wrapNodeAttributes(attributes),
        variableType,
        referenceType,
        std::forward<CompletionToken>(token)
    );
}

/**
 * Add property.
 * @exception BadStatus
 */
template <typename T>
inline NodeId addProperty(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableAttributes& attributes = {}
) {
    return addVariable(
        serverOrClient,
        parentId,
        id,
        browseName,
        attributes,
        VariableTypeId::PropertyType,
        ReferenceTypeId::HasProperty
    );
}

/**
 * Asynchronously add property.
 * @copydetails addProperty
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto addPropertyAsync(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableAttributes& attributes = {},
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addVariableAsync(
        client,
        parentId,
        id,
        browseName,
        attributes,
        VariableTypeId::PropertyType,
        ReferenceTypeId::HasProperty,
        std::forward<CompletionToken>(token)
    );
}

#ifdef UA_ENABLE_METHODCALLS
/**
 * Method callback.
 * @param input Input parameters
 * @param output Output parameters
 */
using MethodCallback = std::function<void(Span<const Variant> input, Span<Variant> output)>;

/**
 * Add method.
 * Callbacks can not be set by clients. Servers can assign callbacks to method nodes afterwards.
 * @exception BadStatus
 */
template <typename T>
NodeId addMethod(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    MethodCallback callback,
    Span<const Argument> inputArguments,
    Span<const Argument> outputArguments,
    const MethodAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasComponent
);

/**
 * Asynchronously add method.
 * @copydetails addMethod
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto addMethodAsync(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    [[maybe_unused]] MethodCallback callback,  // NOLINT
    [[maybe_unused]] Span<const Argument> inputArguments,
    [[maybe_unused]] Span<const Argument> outputArguments,
    const MethodAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasComponent,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addNodeAsync(
        client,
        NodeClass::Method,
        parentId,
        id,
        browseName,
        detail::wrapNodeAttributes(attributes),
        {},
        referenceType,
        std::forward<CompletionToken>(token)
    );
}
#endif

/**
 * Add object type.
 * @exception BadStatus
 */
template <typename T>
inline NodeId addObjectType(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectTypeAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasSubtype
) {
    return addNode(
        serverOrClient,
        NodeClass::ObjectType,
        parentId,
        id,
        browseName,
        detail::wrapNodeAttributes(attributes),
        {},
        referenceType
    );
}

/**
 * Asynchronously add object type.
 * @copydetails addObjectType
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto addObjectTypeAsync(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectTypeAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasSubtype,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addNodeAsync(
        client,
        NodeClass::ObjectType,
        parentId,
        id,
        browseName,
        detail::wrapNodeAttributes(attributes),
        {},
        referenceType,
        std::forward<CompletionToken>(token)
    );
}

/**
 * Add variable type.
 * @exception BadStatus
 */
template <typename T>
inline NodeId addVariableType(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableTypeAttributes& attributes = {},
    const NodeId& variableType = VariableTypeId::BaseDataVariableType,
    const NodeId& referenceType = ReferenceTypeId::HasSubtype
) {
    return addNode(
        serverOrClient,
        NodeClass::VariableType,
        parentId,
        id,
        browseName,
        detail::wrapNodeAttributes(attributes),
        variableType,
        referenceType
    );
}

/**
 * Asynchronously add variable type.
 * @copydetails addVariableType
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto addVariableTypeAsync(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableTypeAttributes& attributes = {},
    const NodeId& variableType = VariableTypeId::BaseDataVariableType,
    const NodeId& referenceType = ReferenceTypeId::HasSubtype,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addNodeAsync(
        client,
        NodeClass::VariableType,
        parentId,
        id,
        browseName,
        detail::wrapNodeAttributes(attributes),
        variableType,
        referenceType,
        std::forward<CompletionToken>(token)
    );
}

/**
 * Add reference type.
 * @exception BadStatus
 */
template <typename T>
inline NodeId addReferenceType(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ReferenceTypeAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasSubtype
) {
    return addNode(
        serverOrClient,
        NodeClass::ReferenceType,
        parentId,
        id,
        browseName,
        detail::wrapNodeAttributes(attributes),
        {},
        referenceType
    );
}

/**
 * Asynchronously add reference type.
 * @copydetails addReferenceType
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto addReferenceTypeAsync(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ReferenceTypeAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasSubtype,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addNodeAsync(
        client,
        NodeClass::ReferenceType,
        parentId,
        id,
        browseName,
        detail::wrapNodeAttributes(attributes),
        {},
        referenceType,
        std::forward<CompletionToken>(token)
    );
}

/**
 * Add data type.
 * @exception BadStatus
 */
template <typename T>
inline NodeId addDataType(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const DataTypeAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasSubtype
) {
    return addNode(
        serverOrClient,
        NodeClass::DataType,
        parentId,
        id,
        browseName,
        detail::wrapNodeAttributes(attributes),
        {},
        referenceType
    );
}

/**
 * Asynchronously add data type.
 * @copydetails addDataType
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto addDataTypeAsync(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const DataTypeAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasSubtype,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addNodeAsync(
        client,
        NodeClass::DataType,
        parentId,
        id,
        browseName,
        detail::wrapNodeAttributes(attributes),
        {},
        referenceType,
        std::forward<CompletionToken>(token)
    );
}

/**
 * Add view.
 * @exception BadStatus
 */
template <typename T>
inline NodeId addView(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ViewAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::Organizes
) {
    return addNode(
        serverOrClient,
        NodeClass::View,
        parentId,
        id,
        browseName,
        detail::wrapNodeAttributes(attributes),
        {},
        referenceType
    );
}

/**
 * Asynchronously add view.
 * @copydetails addView
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto addViewAsync(
    Client& client,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ViewAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::Organizes,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addNodeAsync(
        client,
        NodeClass::View,
        parentId,
        id,
        browseName,
        detail::wrapNodeAttributes(attributes),
        {},
        referenceType,
        std::forward<CompletionToken>(token)
    );
}

/**
 * Add reference.
 * @exception BadStatus
 */
template <typename T>
void addReference(
    T& serverOrClient,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool forward = true
);

/**
 * Asynchronously add reference.
 * @copydetails addReference
 */
template <typename CompletionToken = DefaultCompletionToken>
auto addReferenceAsync(
    Client& client,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool forward = true,
    CompletionToken&& token = DefaultCompletionToken()
) {
    UA_AddReferencesItem item{};
    item.sourceNodeId = sourceId;
    item.referenceTypeId = referenceType;
    item.isForward = forward;
    item.targetServerUri = UA_STRING_NULL;
    item.targetNodeId.nodeId = targetId;
    UA_AddReferencesRequest request{};
    request.referencesToAddSize = 1;
    request.referencesToAdd = &item;
    return detail::sendRequest<UA_AddReferencesRequest, UA_AddReferencesResponse>(
        client,
        request,
        [](UA_AddReferencesResponse& response) {
            detail::throwOnBadStatus(detail::getSingleResult(response));
        },
        std::forward<CompletionToken>(token)
    );
}

/**
 * Add modelling rule.
 * @exception BadStatus
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/6.4.4
 */
template <typename T>
inline void addModellingRule(T& serverOrClient, const NodeId& id, ModellingRule rule) {
    return addReference(
        serverOrClient,
        id,
        {0, static_cast<uint32_t>(rule)},
        ReferenceTypeId::HasModellingRule,
        true
    );
}

/**
 * Asynchronously add modelling rule.
 * @copydetails addModellingRule
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto addModellingRuleAsync(
    Client& client,
    const NodeId& id,
    ModellingRule rule,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addReferenceAsync(
        client,
        id,
        {0, static_cast<uint32_t>(rule)},
        ReferenceTypeId::HasModellingRule,
        true,
        std::forward<CompletionToken>(token)
    );
}

/**
 * Delete node.
 * @exception BadStatus
 */
template <typename T>
void deleteNode(T& serverOrClient, const NodeId& id, bool deleteReferences = true);

/**
 * Asynchronously delete node.
 * @copydetails deleteNode
 */
template <typename CompletionToken = DefaultCompletionToken>
auto deleteNodeAsync(
    Client& client,
    const NodeId& id,
    bool deleteReferences = true,
    CompletionToken&& token = DefaultCompletionToken()
) {
    UA_DeleteNodesItem item{};
    item.nodeId = id;
    item.deleteTargetReferences = deleteReferences;
    UA_DeleteNodesRequest request{};
    request.nodesToDeleteSize = 1;
    request.nodesToDelete = &item;
    return detail::sendRequest<UA_DeleteNodesRequest, UA_DeleteNodesResponse>(
        client,
        request,
        [](UA_DeleteNodesResponse& response) {
            detail::throwOnBadStatus(detail::getSingleResult(response));
        },
        std::forward<CompletionToken>(token)
    );
}

/**
 * Delete reference.
 * @exception BadStatus
 */
template <typename T>
void deleteReference(
    T& serverOrClient,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool isForward,
    bool deleteBidirectional
);

/**
 * Asynchronously delete reference.
 * @copydetails deleteReference
 */
template <typename CompletionToken = DefaultCompletionToken>
auto deleteReferenceAsync(
    Client& client,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool isForward,
    bool deleteBidirectional,
    CompletionToken&& token = DefaultCompletionToken()
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
    return detail::sendRequest<UA_DeleteReferencesRequest, UA_DeleteReferencesResponse>(
        client,
        request,
        [](UA_DeleteReferencesResponse& response) {
            detail::throwOnBadStatus(detail::getSingleResult(response));
        },
        std::forward<CompletionToken>(token)
    );
}

/**
 * @}
 */

}  // namespace opcua::services
