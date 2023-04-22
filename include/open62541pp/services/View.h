#pragma once

#include <vector>

#include "open62541pp/Common.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/Composed.h"
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
 * Discover the references of a specified node.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.2
 * @ingroup View
 */
template <typename T>
BrowseResult browse(T& serverOrClient, const BrowseDescription& bd, uint32_t maxReferences = 0);

/**
 * Request the next step of browse or browseNext response.
 * The response might get split up if the information is too large to be sent in a single response.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.3
 * @ingroup View
 */
template <typename T>
BrowseResult browseNext(T& serverOrClient, const ByteString& continuationPoint);

/**
 * Get a child specified by its path from this node (only local nodes).
 * @exception BadStatus If path not found (BadNoMatch)
 * @ingroup View
 */
NodeId browseChild(Server& server, const NodeId& origin, const std::vector<QualifiedName>& path);

}  // namespace opcua::services
