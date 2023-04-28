#pragma once

#include <string_view>

#include "open62541pp/Common.h"
#include "open62541pp/types/NodeId.h"

// forward declarations
namespace opcua {
class Client;
class Server;
}  // namespace opcua

namespace opcua::services {

/**
 * @defgroup NodeManagement Node management service set
 * Add/delete nodes and references
 * @ingroup Services
 */

/**
 * Add child object.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
void addObject(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& objectType = ObjectTypeId::BaseObjectType,
    const NodeId& referenceType = ReferenceTypeId::HasComponent
);

/**
 * Add child folder.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
inline void addFolder(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& referenceType = ReferenceTypeId::HasComponent
) {
    addObject(serverOrClient, parentId, id, browseName, ObjectTypeId::FolderType, referenceType);
}

/**
 * Add child variable.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
void addVariable(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& variableType = VariableTypeId::BaseDataVariableType,
    const NodeId& referenceType = ReferenceTypeId::HasComponent
);

/**
 * Add child property.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
inline void addProperty(
    T& serverOrClient, const NodeId& parentId, const NodeId& id, std::string_view browseName
) {
    addVariable(
        serverOrClient,
        parentId,
        id,
        browseName,
        VariableTypeId::PropertyType,
        ReferenceTypeId::HasProperty
    );
}

/**
 * Add child object type.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
void addObjectType(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& referenceType = ReferenceTypeId::HasSubtype
);

/**
 * Add child variable type.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
void addVariableType(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& variableType = VariableTypeId::BaseDataVariableType,
    const NodeId& referenceType = ReferenceTypeId::HasSubtype
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

// TODO: deleteReferences

}  // namespace opcua::services
