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
class Variant;

}  // namespace opcua

namespace opcua::services {

/**
 * @defgroup NodeManagement NodeManagement service set
 * Add/delete nodes and references.
 * @ingroup Services
 */

/**
 * Add object.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
void addObject(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectAttributes& attributes = {},
    const NodeId& objectType = ObjectTypeId::BaseObjectType,
    const NodeId& referenceType = ReferenceTypeId::HasComponent
);

/**
 * Add folder.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
inline void addFolder(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasComponent
) {
    addObject(
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
 * @ingroup NodeManagement
 */
template <typename T>
void addVariable(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableAttributes& attributes = {},
    const NodeId& variableType = VariableTypeId::BaseDataVariableType,
    const NodeId& referenceType = ReferenceTypeId::HasComponent
);

/**
 * Add property.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
inline void addProperty(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableAttributes& attributes = {}
) {
    addVariable(
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
 * @ingroup NodeManagement
 */
using MethodCallback = std::function<void(Span<const Variant> input, Span<Variant> output)>;

/**
 * Add method.
 * Callbacks can not be set by clients. Servers can assign callbacks to method nodes afterwards.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
void addMethod(
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
 * @ingroup NodeManagement
 */
template <typename T>
void addObjectType(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ObjectTypeAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasSubtype
);

/**
 * Add variable type.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
void addVariableType(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const VariableTypeAttributes& attributes = {},
    const NodeId& variableType = VariableTypeId::BaseDataVariableType,
    const NodeId& referenceType = ReferenceTypeId::HasSubtype
);

/**
 * Add reference type.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
void addReferenceType(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ReferenceTypeAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasSubtype
);

/**
 * Add data type.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
void addDataType(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const DataTypeAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::HasSubtype
);

/**
 * Add view.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
void addView(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ViewAttributes& attributes = {},
    const NodeId& referenceType = ReferenceTypeId::Organizes
);

/**
 * Add reference.
 * @exception BadStatus
 * @ingroup NodeManagement
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
 * @ingroup NodeManagement
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
 * @ingroup NodeManagement
 */
template <typename T>
void deleteNode(T& serverOrClient, const NodeId& id, bool deleteReferences = true);

/**
 * Delete reference.
 * @exception BadStatus
 * @ingroup NodeManagement
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

}  // namespace opcua::services
