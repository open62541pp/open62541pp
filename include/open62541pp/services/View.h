#pragma once

#include <vector>

#include "open62541pp/Common.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/NodeId.h"

// forward declarations
namespace opcua {
class Server;
}  // namespace opcua

namespace opcua::services {

/**
 * @defgroup View View service set
 * Browse the address space / view created by a server.
 * @ingroup Services
 */

/**
 * Get a child specified by its path from this node (only local nodes).
 * @exception BadStatus If path not found (BadNoMatch)
 * @ingroup View
 */
NodeId browseChild(Server& server, const NodeId& origin, const std::vector<QualifiedName>& path);

}  // namespace opcua::services
