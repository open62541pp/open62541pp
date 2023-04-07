#pragma once

#include <string_view>

#include "open62541pp/Common.h"
#include "open62541pp/types/NodeId.h"

// forward declarations
namespace opcua {
class Server;
}  // namespace opcua

namespace opcua::services {

/**
 * @defgroup NodeManagement Node management
 * Add/delete nodes and references
 * @ingroup Services
 */

/**
 * Add child folder to node.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
void addFolder(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    ReferenceType referenceType = ReferenceType::HasComponent
);

/**
 * Add child object to node.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
void addObject(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& objectType = {0, UA_NS0ID_BASEOBJECTTYPE},
    ReferenceType referenceType = ReferenceType::HasComponent
);

/**
 * Add child variable to node.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
void addVariable(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& variableType = {0, UA_NS0ID_BASEDATAVARIABLETYPE},
    ReferenceType referenceType = ReferenceType::HasComponent
);

/**
 * Add child property to node.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
void addProperty(
    Server& server, const NodeId& parentId, const NodeId& id, std::string_view browseName
);

/**
 * Add child object type to node.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
void addObjectType(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    ReferenceType referenceType = ReferenceType::HasSubType
);

/**
 * Add child variable type to node.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
void addVariableType(
    Server& server,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& variableType = {0, UA_NS0ID_BASEDATAVARIABLETYPE},
    ReferenceType referenceType = ReferenceType::HasSubType
);

/**
 * Add reference to node.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
void addReference(
    Server& server,
    const NodeId& sourceId,
    const NodeId& targetId,
    ReferenceType referenceType,
    bool forward = true
);

/**
 * Delete node.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
void deleteNode(Server& server, const NodeId& id, bool deleteReferences = true);

// TODO: deleteReferences

}  // namespace opcua::services
