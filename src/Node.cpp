#include "open62541pp/Node.h"

#include <cassert>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/TypeWrapper.h"

#include "open62541_impl.h"

namespace opcua {

Node::Node(const Server& server, const NodeId& id)  // NOLINT
    : server_(server),
      nodeId_(id) {
    // check if node exists
    {
        NodeId outputNode(UA_NODEID_NULL);
        const auto status = UA_Server_readNodeId(server_.handle(), nodeId_, outputNode.handle());
        detail::throwOnBadStatus(status);
    }
}

Node Node::getChild(const std::vector<QualifiedName>& path) {
    const std::vector<UA_QualifiedName> pathNative(path.begin(), path.end());
    const TypeWrapper<UA_BrowsePathResult, UA_TYPES_BROWSEPATHRESULT> result(
        UA_Server_browseSimplifiedBrowsePath(
            server_.handle(),
            nodeId_,  // origin
            pathNative.size(),  // browse path size
            pathNative.data()  // browse path
        )
    );
    detail::throwOnBadStatus(result->statusCode);
    assert(result->targets != nullptr && result->targetsSize >= 1);  // NOLINT
    const auto id = ExpandedNodeId(result->targets[0].targetId);  // NOLINT
    if (!id.isLocal()) {
        throw BadStatus(UA_STATUSCODE_BADNOMATCH);
    }
    return {server_, id.getNodeId()};
}

/* ---------------------------------------------------------------------------------------------- */

bool operator==(const Node& left, const Node& right) noexcept {
    return (left.getServer() == right.getServer()) && (left.getNodeId() == right.getNodeId());
}

bool operator!=(const Node& left, const Node& right) noexcept {
    return !(left == right);
}

}  // namespace opcua
