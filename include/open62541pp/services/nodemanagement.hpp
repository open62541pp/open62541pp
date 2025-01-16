#pragma once

#include <cstdint>
#include <functional>
#include <string_view>
#include <utility>  // exchange, forward

#include "open62541pp/async.hpp"
#include "open62541pp/common.hpp"  // ModellingRule
#include "open62541pp/config.hpp"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/result.hpp"
#include "open62541pp/services/detail/async_transform.hpp"
#include "open62541pp/services/detail/client_service.hpp"
#include "open62541pp/services/detail/request_handling.hpp"
#include "open62541pp/services/detail/response_handling.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/ua/nodeids.hpp"  // *TypeId
#include "open62541pp/ua/types.hpp"
#include "open62541pp/session.hpp"

namespace opcua {
class Client;
}

namespace opcua::services {

/**
 * @defgroup NodeManagement NodeManagement service set
 * Add/delete nodes and references.
 *
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8
 * @ingroup Services
 * @{
 */

/**
 * @defgroup AddNodes AddNodes service
 * Add nodes into the address space hierarchy.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.2
 * @{
 */

/**
 * Add one or more nodes (client only).
 * @param connection Instance of type Client
 * @param request Add nodes request
 */
AddNodesResponse addNodes(Client& connection, const AddNodesRequest& request) noexcept;

/**
 * @copydoc addNodes
 * @param token @completiontoken{void(AddNodesResponse&)}
 * @return @asyncresult{AddNodesResponse}
 */
template <typename CompletionToken>
auto addNodesAsync(Client& connection, const AddNodesRequest& request, CompletionToken&& token) {
    return detail::sendRequestAsync<AddNodesRequest, AddNodesResponse>(
        connection, request, std::forward<CompletionToken>(token)
    );
}

/**
 * Add a node.
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
 * @copydoc addNode
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken>
auto addNodeAsync(
    Client& connection,
    NodeClass nodeClass,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ExtensionObject& nodeAttributes,
    const NodeId& typeDefinition,
    const NodeId& referenceType,
    CompletionToken&& token
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
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.3
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
 * @copydoc addReferences
 * @param token @completiontoken{void(AddReferencesResponse&)}
 * @return @asyncresult{AddReferencesReponse}
 */
template <typename CompletionToken>
auto addReferencesAsync(
    Client& connection, const AddReferencesRequest& request, CompletionToken&& token
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
    bool forward
) noexcept;

/**
 * @copydoc addReference
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 */
template <typename CompletionToken>
auto addReferenceAsync(
    Client& connection,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool forward,
    CompletionToken&& token
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
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.4
 * @{
 */

/**
 * Delete one or more nodes (client only).
 * @param connection Instance of type Client
 * @param request Delete nodes request
 */
DeleteNodesResponse deleteNodes(Client& connection, const DeleteNodesRequest& request) noexcept;

/**
 * @copydoc deleteNodes
 * @param token @completiontoken{void(DeleteNodesResponse&)}
 * @return @asyncresult{DeleteNodesResponse}
 */
template <typename CompletionToken>
auto deleteNodesAsync(
    Client& connection, const DeleteNodesRequest& request, CompletionToken&& token
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
StatusCode deleteNode(T& connection, const NodeId& id, bool deleteReferences) noexcept;

/**
 * @copydoc deleteNode
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 */
template <typename CompletionToken>
auto deleteNodeAsync(
    Client& connection, const NodeId& id, bool deleteReferences, CompletionToken&& token
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
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.5
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
 * @copydoc deleteReferences
 * @param token @completiontoken{void(DeleteReferencesResponse&)}
 * @return @asyncresult{DeleteReferencesResponse}
 */
template <typename CompletionToken>
auto deleteReferencesAsync(
    Client& connection, const DeleteReferencesRequest& request, CompletionToken&& token
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
 * @copydoc deleteReference
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 */
template <typename CompletionToken>
auto deleteReferenceAsync(
    Client& connection,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool isForward,
    bool deleteBidirectional,
    CompletionToken&& token
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
 * @param connection Instance of type Client (or Server)
 * @param parentId Parent node
 * @param id Requested NodeId of the object node to add
 * @param browseName Browse name
 * @param attributes Object attributes
 * @param objectType NodeId of the object type, e.g. ObjectTypeId::BaseObjectType
 * @param referenceType Hierarchical reference type from the parent node to the new node, e.g.
 *                      ReferenceTypeId::HasComponent
 */
template <typename T>
Result<NodeId> addObject(
    T& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectAttributes& attributes,
    const NodeId& objectType,
    const NodeId& referenceType
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
 * @copydoc addObject
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken>
auto addObjectAsync(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectAttributes& attributes,
    const NodeId& objectType,
    const NodeId& referenceType,
    CompletionToken&& token
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
 * @param connection Instance of type Client (or Server)
 * @param parentId Parent node
 * @param id Requested NodeId of the node to add
 * @param browseName Browse name
 * @param attributes Object attributes
 * @param referenceType Hierarchical reference type from the parent node to the new node, e.g.
 *                      ReferenceTypeId::HasComponent
 */
template <typename T>
Result<NodeId> addFolder(
    T& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectAttributes& attributes,
    const NodeId& referenceType
) noexcept {
    return addObject(
        connection, parentId, id, browseName, attributes, ObjectTypeId::FolderType, referenceType
    );
}

/**
 * @copydoc addFolder
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken>
auto addFolderAsync(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectAttributes& attributes,
    const NodeId& referenceType,
    CompletionToken&& token
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
 * @param connection Instance of type Client (or Server)
 * @param parentId Parent node
 * @param id Requested NodeId of the node to add
 * @param browseName Browse name
 * @param attributes Variable attributes
 * @param variableType NodeId of the variable type, e.g. VariableTypeId::BaseDataVariableType
 * @param referenceType Hierarchical reference type from the parent node to the new node, e.g.
 *                      ReferenceTypeId::HasComponent
 */
template <typename T>
Result<NodeId> addVariable(
    T& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableAttributes& attributes,
    const NodeId& variableType,
    const NodeId& referenceType
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
 * @copydoc addVariable
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken>
auto addVariableAsync(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableAttributes& attributes,
    const NodeId& variableType,
    const NodeId& referenceType,
    CompletionToken&& token
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
    const VariableAttributes& attributes
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
 * @copydoc addProperty
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken>
auto addPropertyAsync(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableAttributes& attributes,
    CompletionToken&& token
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
using MethodCallback = std::function<void(Session& session, const NodeId& methodID, Span<const Variant> input, Span<Variant> output)>;

/**
 * Add method.
 * Callbacks can not be set by clients. Servers can assign callbacks to method nodes afterwards.
 * @param connection Instance of type Client (or Server)
 * @param parentId Parent node
 * @param id Requested NodeId of the node to add
 * @param browseName Browse name
 * @param callback Method callback
 * @param inputArguments Input arguments
 * @param outputArguments Output arguments
 * @param attributes Method attributes
 * @param referenceType Hierarchical reference type from the parent node to the new node, e.g.
 *                      ReferenceTypeId::HasComponent
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
    const MethodAttributes& attributes,
    const NodeId& referenceType
) noexcept;

/**
 * @copydoc addMethod
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken>
auto addMethodAsync(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    [[maybe_unused]] MethodCallback callback,  // NOLINT
    [[maybe_unused]] Span<const Argument> inputArguments,
    [[maybe_unused]] Span<const Argument> outputArguments,
    const MethodAttributes& attributes,
    const NodeId& referenceType,
    CompletionToken&& token
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
 * @param connection Instance of type Client (or Server)
 * @param parentId Parent node
 * @param id Requested NodeId of the node to add
 * @param browseName Browse name
 * @param attributes Object type attributes
 * @param referenceType Hierarchical reference type from the parent node to the new node, e.g.
 *                      ReferenceTypeId::HasSubtype
 */
template <typename T>
Result<NodeId> addObjectType(
    T& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectTypeAttributes& attributes,
    const NodeId& referenceType
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
 * @copydoc addObjectType
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken>
auto addObjectTypeAsync(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectTypeAttributes& attributes,
    const NodeId& referenceType,
    CompletionToken&& token
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
 * @param connection Instance of type Client (or Server)
 * @param parentId Parent node
 * @param id Requested NodeId of the node to add
 * @param browseName Browse name
 * @param attributes Variable type attributes
 * @param variableType NodeId of the variable type, e.g. VariableTypeId::BaseDataVariableType
 * @param referenceType Hierarchical reference type from the parent node to the new node, e.g.
 *                      ReferenceTypeId::HasSubtype
 */
template <typename T>
Result<NodeId> addVariableType(
    T& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableTypeAttributes& attributes,
    const NodeId& variableType,
    const NodeId& referenceType
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
 * @copydoc addVariableType
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken>
auto addVariableTypeAsync(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableTypeAttributes& attributes,
    const NodeId& variableType,
    const NodeId& referenceType,
    CompletionToken&& token
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
 * @param connection Instance of type Client (or Server)
 * @param parentId Parent node
 * @param id Requested NodeId of the node to add
 * @param browseName Browse name
 * @param attributes Reference type attributes
 * @param referenceType Hierarchical reference type from the parent node to the new node, e.g.
 *                      ReferenceTypeId::HasSubtype
 */
template <typename T>
Result<NodeId> addReferenceType(
    T& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ReferenceTypeAttributes& attributes,
    const NodeId& referenceType
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
 * @copydoc addReferenceType
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken>
auto addReferenceTypeAsync(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ReferenceTypeAttributes& attributes,
    const NodeId& referenceType,
    CompletionToken&& token
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
 * @param connection Instance of type Client (or Server)
 * @param parentId Parent node
 * @param id Requested NodeId of the node to add
 * @param browseName Browse name
 * @param attributes Data type attributes
 * @param referenceType Hierarchical reference type from the parent node to the new node, e.g.
 *                      ReferenceTypeId::HasSubtype
 */
template <typename T>
Result<NodeId> addDataType(
    T& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const DataTypeAttributes& attributes,
    const NodeId& referenceType
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
 * @copydoc addDataType
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken>
auto addDataTypeAsync(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const DataTypeAttributes& attributes,
    const NodeId& referenceType,
    CompletionToken&& token
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
 * @param connection Instance of type Client (or Server)
 * @param parentId Parent node
 * @param id Requested NodeId of the node to add
 * @param browseName Browse name
 * @param attributes View attributes
 * @param referenceType Hierarchical reference type from the parent node to the new node, e.g.
 *                      ReferenceTypeId::Organizes
 */
template <typename T>
Result<NodeId> addView(
    T& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ViewAttributes& attributes,
    const NodeId& referenceType
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
 * @copydoc addView
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 */
template <typename CompletionToken>
auto addViewAsync(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ViewAttributes& attributes,
    const NodeId& referenceType,
    CompletionToken&& token
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
 * @copydoc addModellingRule
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 */
template <typename CompletionToken>
auto addModellingRuleAsync(
    Client& connection, const NodeId& id, ModellingRule rule, CompletionToken&& token
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
