#include "open62541pp/services/View.h"

#include "open62541pp/Client.h"
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
BrowseResult browse<Client>(Client& client, const BrowseDescription& bd, uint32_t maxReferences) {
    UA_BrowseRequest request{};
    request.requestedMaxReferencesPerNode = maxReferences;
    request.nodesToBrowseSize = 1;
    // NOLINTNEXTLINE, won't be modified
    request.nodesToBrowse = const_cast<UA_BrowseDescription*>(bd.handle());

    using BrowseResponse = TypeWrapper<UA_BrowseResponse, UA_TYPES_BROWSERESPONSE>;
    BrowseResponse response = UA_Client_Service_browse(client.handle(), request);
    detail::throwOnBadStatus(response->responseHeader.serviceResult);

    if (response->resultsSize != 1) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }

    BrowseResult result;
    result.swap(*response->results);
    return result;
}

template <>
BrowseResult browseNext<Server>(
    Server& server, bool releaseContinuationPoint, const ByteString& continuationPoint
) {
    BrowseResult result = UA_Server_browseNext(
        server.handle(), releaseContinuationPoint, continuationPoint.handle()
    );
    detail::throwOnBadStatus(result->statusCode);
    return result;
}

template <>
BrowseResult browseNext<Client>(
    Client& client, bool releaseContinuationPoint, const ByteString& continuationPoint
) {
    UA_BrowseNextRequest request{};
    request.releaseContinuationPoints = releaseContinuationPoint;
    request.continuationPointsSize = 1;
    // NOLINTNEXTLINE, won't be modified
    request.continuationPoints = const_cast<UA_ByteString*>(continuationPoint.handle());

    using BrowseNextResponse = TypeWrapper<UA_BrowseNextResponse, UA_TYPES_BROWSENEXTRESPONSE>;
    BrowseNextResponse response = UA_Client_Service_browseNext(client.handle(), request);
    detail::throwOnBadStatus(response->responseHeader.serviceResult);

    if (response->resultsSize != 1) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }

    BrowseResult result;
    result.swap(*response->results);
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
