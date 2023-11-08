#pragma once

#include <cstdint>
#include <functional>
#include <string_view>

#include "open62541pp/Common.h"  // ModellingRule
#include "open62541pp/Config.h"
#include "open62541pp/NodeIds.h"  // *TypeId
#include "open62541pp/Span.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/NodeId.h"

// forward declarations
namespace opcua {
class Client;
class Variant;
}  // namespace opcua

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
 * Add one or more references (client only).
 */
AddReferencesResponse addReferences(Client& client, const AddReferencesRequest& request);

/**
 * Delete one or more nodes (client only).
 */
DeleteNodesResponse deleteNodes(Client& client, const DeleteNodesRequest& request);

/**
 * Delete one or more references (client only).
 */
DeleteReferencesResponse deleteReferences(Client& client, const DeleteReferencesRequest& request);

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
        ExtensionObject::fromDecoded(const_cast<ObjectAttributes&>(attributes)),  // NOLINT
        objectType,
        referenceType
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
        ExtensionObject::fromDecoded(const_cast<VariableAttributes&>(attributes)),  // NOLINT
        variableType,
        referenceType
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
        ExtensionObject::fromDecoded(const_cast<ObjectTypeAttributes&>(attributes)),  // NOLINT
        {},
        referenceType
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
        ExtensionObject::fromDecoded(const_cast<VariableTypeAttributes&>(attributes)),  // NOLINT
        variableType,
        referenceType
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
        ExtensionObject::fromDecoded(const_cast<ReferenceTypeAttributes&>(attributes)),  // NOLINT
        {},
        referenceType
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
        ExtensionObject::fromDecoded(const_cast<DataTypeAttributes&>(attributes)),  // NOLINT
        {},
        referenceType
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
        ExtensionObject::fromDecoded(const_cast<ViewAttributes&>(attributes)),  // NOLINT
        {},
        referenceType
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
 * Add modelling rule.
 * @exception BadStatus
 */
template <typename T>
inline void addModellingRule(T& serverOrClient, const NodeId& id, ModellingRule rule) {
    addReference(
        serverOrClient,
        id,
        {0, static_cast<uint32_t>(rule)},
        ReferenceTypeId::HasModellingRule,
        true
    );
}

/**
 * Delete node.
 * @exception BadStatus
 */
template <typename T>
void deleteNode(T& serverOrClient, const NodeId& id, bool deleteReferences = true);

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
 * @}
 */

}  // namespace opcua::services
