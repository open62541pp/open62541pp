#include "open62541pp/services/View.h"

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"

#include "../open62541_impl.h"

namespace opcua::services {

template <>
BrowseResult browse<Server>(Server& server, const BrowseDescription& bd, uint32_t maxReferences) {
    BrowseResult result = UA_Server_browse(server.handle(), maxReferences, bd.handle());
    detail::throwOnBadStatus(result->statusCode);
    return result;
}

template <>
BrowseResult browseNext<Server>(Server& server, const ByteString& continuationPoint) {
    BrowseResult result = UA_Server_browseNext(server.handle(), false, continuationPoint.handle());
    detail::throwOnBadStatus(result->statusCode);
    return result;
}

NodeId browseChild(Server& server, const NodeId& origin, const std::vector<QualifiedName>& path) {
    const std::vector<UA_QualifiedName> pathNative(path.begin(), path.end());
    const TypeWrapper<UA_BrowsePathResult, UA_TYPES_BROWSEPATHRESULT> result(
        UA_Server_browseSimplifiedBrowsePath(
            server.handle(),
            origin,  // origin
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
    return id.getNodeId();
}

}  // namespace opcua::services
