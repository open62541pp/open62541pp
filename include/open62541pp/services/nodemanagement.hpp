#pragma once

#include <cstdint>
#include <functional>
#include <string_view>
#include <utility>  // exchange, forward

#include "open62541pp/async.hpp"
#include "open62541pp/common.hpp"  // ModellingRule
#include "open62541pp/config.hpp"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/nodeids.hpp"  // *TypeId
#include "open62541pp/result.hpp"
#include "open62541pp/services/detail/async_transform.hpp"
#include "open62541pp/services/detail/client_service.hpp"
#include "open62541pp/services/detail/request_handling.hpp"
#include "open62541pp/services/detail/response_handling.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/types_composed.hpp"

namespace opcua {
class Client;
}

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
 * @defgroup AddNodes AddNodes service
 * Add nodes into the address space hierarchy.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.2
 * @{
 */

/**
 * Add one or more nodes (client only).
 * @param connection Instance of type Client
 * @param request Add nodes request
 */
AddNodesResponse addNodes(Client& connection, const AddNodesRequest& request) noexcept;

/**
 * Asynchronously add one or more nodes (client only).
 * @copydetails addNodes
 * @param token @completiontoken{void(AddNodesResponse&)}
 * @return @asyncresult{AddNodesResponse}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto addNodesAsync(
    Client& connection,
    const AddNodesRequest& request,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequestAsync<AddNodesRequest, AddNodesResponse>(
        connection, request, std::forward<CompletionToken>(token)
    );
}

/**
 * Add a node.
 * @return Server-assigned NodeId of the added node
 * @param connection Instance of type Client (or Server)
 * @param nodeClass Node class
 * @param parentId Parent node
 * @param id Requested NodeId of the node to add
 * @param browseName Browse name
 * @param nodeAttributes Node attributes
 * @param typeDefinition NodeId of the type
 * @param referenceType Hierarchical reference type from the parent node to the new node
 */
template <typename T>
Result<NodeId> addNode(
    T& connection,
    NodeClass nodeClass,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ExtensionObject& nodeAttributes,
    const NodeId& typeDefinition,
    const NodeId& referenceType
) noexcept;

/**
 * Asynchronously add a node.
 * @copydetails addNode
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto addNodeAsync(
    Client& connection,
    NodeClass nodeClass,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ExtensionObject& nodeAttributes,
    const NodeId& typeDefinition,
    const NodeId& referenceType,
    CompletionToken&& token = DefaultCompletionToken()
) {
    auto item = detail::createAddNodesItem(
        parentId, referenceType, id, browseName, nodeClass, nodeAttributes, typeDefinition
    );
    const auto request = detail::createAddNodesRequest(item);
    return addNodesAsync(
        connection,
        asWrapper<AddNodesRequest>(request),
        detail::TransformToken(
            [](UA_AddNodesResponse& response) {
                return detail::getSingleResultRef(response).andThen(detail::getAddedNodeId);
            },
            std::forward<CompletionToken>(token)
        )
    );
}

/**
 * @}
 * @defgroup AddReferences AddReferences service
 * Add references to nodes.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.3
 * @{
 */

/**
 * Add one or more references (client only).
 * @param connection Instance of type Client
 * @param request Add references request
 */
AddReferencesResponse addReferences(
    Client& connection, const AddReferencesRequest& request
) noexcept;

/**
 * Asynchronously add one or more references (client only).
 * @copydetails addReferences
 * @param token @completiontoken{void(AddReferencesResponse&)}
 * @return @asyncresult{AddReferencesReponse}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto addReferencesAsync(
    Client& connection,
    const AddReferencesRequest& request,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequestAsync<AddReferencesRequest, AddReferencesResponse>(
        connection, request, std::forward<CompletionToken>(token)
    );
}

/**
 * Add reference.
 * @param connection Instance of type Client (or Server)
 * @param sourceId Node to which the reference is to be added
 * @param targetId Target node
 * @param referenceType NodeId of the reference type that defines the reference
 * @param forward Create a forward reference if `true` or a inverse reference if `false`
 */
template <typename T>
StatusCode addReference(
    T& connection,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool forward = true
) noexcept;

/**
 * Asynchronously add reference.
 * @copydetails addReference
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto addReferenceAsync(
    Client& connection,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool forward = true,
    CompletionToken&& token = DefaultCompletionToken()
) {
    auto item = detail::createAddReferencesItem(sourceId, referenceType, forward, targetId);
    const auto request = detail::createAddReferencesRequest(item);
    return addReferencesAsync(
        connection,
        asWrapper<AddReferencesRequest>(request),
        detail::TransformToken(
            detail::getSingleStatus<AddReferencesResponse>, std::forward<CompletionToken>(token)
        )
    );
}

/**
 * @}
 * @defgroup DeleteNodes DeleteNodes service
 * Delete nodes from the address space.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.4
 * @{
 */

/**
 * Delete one or more nodes (client only).
 * @param connection Instance of type Client
 * @param request Delete nodes request
 */
DeleteNodesResponse deleteNodes(Client& connection, const DeleteNodesRequest& request) noexcept;

/**
 * Asynchronously delete one or more nodes (client only).
 * @copydetails deleteNodes
 * @param token @completiontoken{void(DeleteNodesResponse&)}
 * @return @asyncresult{DeleteNodesResponse}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto deleteNodesAsync(
    Client& connection,
    const DeleteNodesRequest& request,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequestAsync<DeleteNodesRequest, DeleteNodesResponse>(
        connection, request, std::forward<CompletionToken>(token)
    );
}

/**
 * Delete node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to delete
 * @param deleteReferences Delete references in target nodes that reference the node to delete
 */
template <typename T>
StatusCode deleteNode(T& connection, const NodeId& id, bool deleteReferences = true) noexcept;

/**
 * Asynchronously delete node.
 * @copydetails deleteNode
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto deleteNodeAsync(
    Client& connection,
    const NodeId& id,
    bool deleteReferences = true,
    CompletionToken&& token = DefaultCompletionToken()
) {
    auto item = detail::createDeleteNodesItem(id, deleteReferences);
    const auto request = detail::createDeleteNodesRequest(item);
    return deleteNodesAsync(
        connection,
        asWrapper<DeleteNodesRequest>(request),
        detail::TransformToken(
            detail::getSingleStatus<DeleteNodesResponse>, std::forward<CompletionToken>(token)
        )
    );
}

/**
 * @}
 * @defgroup DeleteReferences DeleteReferences service
 * Delete references from nodes.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.5
 * @{
 */

/**
 * Delete one or more references (client only).
 * @param connection Instance of type Client
 * @param request Delete references request
 */
DeleteReferencesResponse deleteReferences(
    Client& connection, const DeleteReferencesRequest& request
) noexcept;

/**
 * Asynchronously delete one or more references (client only).
 * @copydetails deleteReferences
 * @param token @completiontoken{void(DeleteReferencesResponse&)}
 * @return @asyncresult{DeleteReferencesResponse}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto deleteReferencesAsync(
    Client& connection,
    const DeleteReferencesRequest& request,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequestAsync<DeleteReferencesRequest, DeleteReferencesResponse>(
        connection, request, std::forward<CompletionToken>(token)
    );
}

/**
 * Delete reference.
 * @param connection Instance of type Client (or Server)
 * @param sourceId Node that contains the reference to delete
 * @param targetId Target node of the reference to delete
 * @param referenceType NodeId of the reference type that defines the reference to delete
 * @param isForward Delete the forward reference if `true`, delete the inverse reference if `false`
 * @param deleteBidirectional Delete the specified and opposite reference from the target node
 */
template <typename T>
StatusCode deleteReference(
    T& connection,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool isForward,
    bool deleteBidirectional
) noexcept;

/**
 * Asynchronously delete reference.
 * @copydetails deleteReference
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto deleteReferenceAsync(
    Client& connection,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool isForward,
    bool deleteBidirectional,
    CompletionToken&& token = DefaultCompletionToken()
) {
    auto item = detail::createDeleteReferencesItem(
        sourceId, referenceType, isForward, targetId, deleteBidirectional
    );
    const auto request = detail::createDeleteReferencesRequest(item);
    return deleteReferencesAsync(
        connection,
        asWrapper<DeleteReferencesRequest>(request),
        detail::TransformToken(
            detail::getSingleStatus<DeleteReferencesResponse>, std::forward<CompletionToken>(token)
        )
    );
}

/**
 * @}
 */

/* ------------------------------- Specialized (inline) functions ------------------------------- */

/**
 * @addtogroup AddNodes
 * @{
 */

/**
 * Add object.
 * @return Server-assigned NodeId of the added object node
 * @param connection Instance of type Client (or Server)
 * @param parentId Parent node
 * @param id Requested NodeId of the object node to add
 * @param browseName Browse name
 * @param attributes Object attributes
 * @param objectType NodeId of the object type
 * @param referenceType Hierarchical reference type from the parent node to the new node
 */
template <typename T>
Result<NodeId> addObject(
    T& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectAttributes& attributes = {},
    const NodeId& objectType = ObjectTypeId::BaseObjectType,
    const NodeId& referenceType = ReferenceTypeId::HasComponent
) noexcept {
    return addNode(
        connection,
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
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto addObjectAsync(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectAttributes& attributes = {},
    const NodeId& objectType = ObjectTypeId::BaseObjectType,
    const NodeId& referenceType = ReferenceTypeId::HasComponent,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addNodeAsync(
        connection,
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
 * @return Server-assigned NodeId of the added folder node
 * @param connection Instance of type Client (or Server)
 * @param parentId Parent node
 * @param id Requested NodeId of the node to add
 * @param browseName Browse name
 * @param attributes Object attributes
 * @param referenceType Hierarchical reference type from the parent node to the new node
 */
template <typename T>
Result<NodeId> addFolder(
    T& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasComponent
) noexcept {
    return addObject(
        connection, parentId, id, browseName, attributes, ObjectTypeId::FolderType, referenceType
    );
}

/**
 * Asynchronously add folder.
 * @copydetails addFolder
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto addFolderAsync(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasComponent,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addObjectAsync(
        connection,
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
 * @return Server-assigned NodeId of the added variable node
 * @param connection Instance of type Client (or Server)
 * @param parentId Parent node
 * @param id Requested NodeId of the node to add
 * @param browseName Browse name
 * @param attributes Variable attributes
 * @param variableType NodeId of the variable type
 * @param referenceType Hierarchical reference type from the parent node to the new node
 */
template <typename T>
Result<NodeId> addVariable(
    T& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableAttributes& attributes = {},
    const NodeId& variableType = VariableTypeId::BaseDataVariableType,
    const NodeId& referenceType = ReferenceTypeId::HasComponent
) noexcept {
    return addNode(
        connection,
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
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto addVariableAsync(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableAttributes& attributes = {},
    const NodeId& variableType = VariableTypeId::BaseDataVariableType,
    const NodeId& referenceType = ReferenceTypeId::HasComponent,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addNodeAsync(
        connection,
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
 * @return Server-assigned NodeId of the added property node
 * @param connection Instance of type Client (or Server)
 * @param parentId Parent node
 * @param id Requested NodeId of the node to add
 * @param browseName Browse name
 * @param attributes Property attributes
 */
template <typename T>
Result<NodeId> addProperty(
    T& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableAttributes& attributes = {}
) noexcept {
    return addVariable(
        connection,
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
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto addPropertyAsync(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableAttributes& attributes = {},
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addVariableAsync(
        connection,
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
 * @return Server-assigned NodeId of the added method node
 * @param connection Instance of type Client (or Server)
 * @param parentId Parent node
 * @param id Requested NodeId of the node to add
 * @param browseName Browse name
 * @param callback Method callback
 * @param inputArguments Input arguments
 * @param outputArguments Output arguments
 * @param attributes Method attributes
 * @param referenceType Hierarchical reference type from the parent node to the new node
 */
template <typename T>
Result<NodeId> addMethod(
    T& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    MethodCallback callback,
    Span<const Argument> inputArguments,
    Span<const Argument> outputArguments,
    const MethodAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasComponent
) noexcept;

/**
 * Asynchronously add method.
 * @copydetails addMethod
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto addMethodAsync(
    Client& connection,
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
        connection,
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
 * @return Server-assigned NodeId of the added object type node
 * @param connection Instance of type Client (or Server)
 * @param parentId Parent node
 * @param id Requested NodeId of the node to add
 * @param browseName Browse name
 * @param attributes Object type attributes
 * @param referenceType Hierarchical reference type from the parent node to the new node
 */
template <typename T>
Result<NodeId> addObjectType(
    T& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectTypeAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasSubtype
) noexcept {
    return addNode(
        connection,
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
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto addObjectTypeAsync(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectTypeAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasSubtype,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addNodeAsync(
        connection,
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
 * @return Server-assigned NodeId of the added variable type node
 * @param connection Instance of type Client (or Server)
 * @param parentId Parent node
 * @param id Requested NodeId of the node to add
 * @param browseName Browse name
 * @param attributes Variable type attributes
 * @param variableType NodeId of the variable type
 * @param referenceType Hierarchical reference type from the parent node to the new node
 */
template <typename T>
Result<NodeId> addVariableType(
    T& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableTypeAttributes& attributes = {},
    const NodeId& variableType = VariableTypeId::BaseDataVariableType,
    const NodeId& referenceType = ReferenceTypeId::HasSubtype
) noexcept {
    return addNode(
        connection,
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
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto addVariableTypeAsync(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableTypeAttributes& attributes = {},
    const NodeId& variableType = VariableTypeId::BaseDataVariableType,
    const NodeId& referenceType = ReferenceTypeId::HasSubtype,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addNodeAsync(
        connection,
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
 * @return Server-assigned NodeId of the added reference type node
 * @param connection Instance of type Client (or Server)
 * @param parentId Parent node
 * @param id Requested NodeId of the node to add
 * @param browseName Browse name
 * @param attributes Reference type attributes
 * @param referenceType Hierarchical reference type from the parent node to the new node
 */
template <typename T>
Result<NodeId> addReferenceType(
    T& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ReferenceTypeAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasSubtype
) noexcept {
    return addNode(
        connection,
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
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto addReferenceTypeAsync(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ReferenceTypeAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasSubtype,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addNodeAsync(
        connection,
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
 * @return Server-assigned NodeId of the added data type node
 * @param connection Instance of type Client (or Server)
 * @param parentId Parent node
 * @param id Requested NodeId of the node to add
 * @param browseName Browse name
 * @param attributes Data type attributes
 * @param referenceType Hierarchical reference type from the parent node to the new node
 */
template <typename T>
Result<NodeId> addDataType(
    T& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const DataTypeAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasSubtype
) noexcept {
    return addNode(
        connection,
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
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto addDataTypeAsync(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const DataTypeAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasSubtype,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addNodeAsync(
        connection,
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
 * @return Server-assigned NodeId of the added view node
 * @param connection Instance of type Client (or Server)
 * @param parentId Parent node
 * @param id Requested NodeId of the node to add
 * @param browseName Browse name
 * @param attributes View attributes
 * @param referenceType Hierarchical reference type from the parent node to the new node
 */
template <typename T>
Result<NodeId> addView(
    T& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ViewAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::Organizes
) noexcept {
    return addNode(
        connection,
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
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto addViewAsync(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ViewAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::Organizes,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addNodeAsync(
        connection,
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
 * @}
 * @addtogroup AddReferences
 * @{
 */

/**
 * Add modelling rule.
 * @param connection Instance of type Client (or Server)
 * @param id Node
 * @param rule Modelling rule to add
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/6.4.4
 */
template <typename T>
StatusCode addModellingRule(T& connection, const NodeId& id, ModellingRule rule) noexcept {
    return addReference(
        connection, id, {0, static_cast<uint32_t>(rule)}, ReferenceTypeId::HasModellingRule, true
    );
}

/**
 * Asynchronously add modelling rule.
 * @copydetails addModellingRule
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto addModellingRuleAsync(
    Client& connection,
    const NodeId& id,
    ModellingRule rule,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return addReferenceAsync(
        connection,
        id,
        {0, static_cast<uint32_t>(rule)},
        ReferenceTypeId::HasModellingRule,
        true,
        std::forward<CompletionToken>(token)
    );
}

/**
 * @}
 * @}
 */

}  // namespace opcua::services
