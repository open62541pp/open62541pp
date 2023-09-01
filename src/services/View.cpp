#include "open62541pp/services/View.h"

#include <algorithm>  // transform
#include <cstddef>

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/NodeIds.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/helper.h"  // getUaDataType
#include "open62541pp/types/Builtin.h"

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

    using Response = TypeWrapper<UA_BrowseResponse, UA_TYPES_BROWSERESPONSE>;
    Response response = UA_Client_Service_browse(client.handle(), request);
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

    using Response = TypeWrapper<UA_BrowseNextResponse, UA_TYPES_BROWSENEXTRESPONSE>;
    Response response = UA_Client_Service_browseNext(client.handle(), request);
    detail::throwOnBadStatus(response->responseHeader.serviceResult);
    if (response->resultsSize != 1) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }

    BrowseResult result;
    result.swap(*response->results);
    return result;
}

template <typename T>
std::vector<ReferenceDescription> browseAll(
    T& serverOrClient, const BrowseDescription& bd, uint32_t maxReferences
) {
    auto response = browse(serverOrClient, bd, maxReferences);
    std::vector<ReferenceDescription> refs(response.getReferences());
    while (!response.getContinuationPoint().empty()) {
        const bool release = (refs.size() >= maxReferences);
        response = browseNext(serverOrClient, release, response.getContinuationPoint());
        auto refsNext = response.getReferences();
        refs.insert(refs.end(), refsNext.begin(), refsNext.end());
    }
    if ((maxReferences > 0) && (refs.size() > maxReferences)) {
        refs.resize(maxReferences);
    }
    return refs;
}

std::vector<ExpandedNodeId> browseRecursive(Server& server, const BrowseDescription& bd) {
    UA_ExpandedNodeId* resultsNative{};
    size_t resultsSize{};
    const auto status = UA_Server_browseRecursive(
        server.handle(), bd.handle(), &resultsSize, &resultsNative
    );
    detail::throwOnBadStatus(status);
    std::vector<ExpandedNodeId> results(resultsSize);
    for (size_t i = 0; i < resultsSize; ++i) {
        results[i].swap(resultsNative[i]);  // NOLINT
    }
    UA_Array_delete(resultsNative, resultsSize, &detail::getUaDataType(UA_TYPES_EXPANDEDNODEID));
    return results;
}

template <>
BrowsePathResult translateBrowsePathToNodeIds<Server>(
    Server& server, const BrowsePath& browsePath
) {
    BrowsePathResult result = UA_Server_translateBrowsePathToNodeIds(
        server.handle(), browsePath.handle()
    );
    detail::throwOnBadStatus(result->statusCode);
    return result;
}

template <>
BrowsePathResult translateBrowsePathToNodeIds<Client>(
    Client& client, const BrowsePath& browsePath
) {
    UA_TranslateBrowsePathsToNodeIdsRequest request{};
    request.browsePathsSize = 1;
    // NOLINTNEXTLINE, won't be modified
    request.browsePaths = const_cast<UA_BrowsePath*>(browsePath.handle());

    using Response = TypeWrapper<
        UA_TranslateBrowsePathsToNodeIdsResponse,
        UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE>;
    Response response = UA_Client_Service_translateBrowsePathsToNodeIds(client.handle(), request);
    detail::throwOnBadStatus(response->responseHeader.serviceResult);
    if (response->resultsSize != 1) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }

    BrowsePathResult result;
    result.swap(*response->results);
    return result;
}

template <typename T>
BrowsePathResult browseSimplifiedBrowsePath(
    T& serverOrClient, const NodeId& origin, Span<const QualifiedName> browsePath
) {
    std::vector<RelativePathElement> relativePathElements(browsePath.size());
    std::transform(
        browsePath.begin(),
        browsePath.end(),
        relativePathElements.begin(),
        [](const auto& qn) {
            return RelativePathElement(ReferenceTypeId::HierarchicalReferences, false, true, qn);
        }
    );
    const BrowsePath bp(origin, RelativePath(relativePathElements));
    return translateBrowsePathToNodeIds(serverOrClient, bp);
}

// explicit template instantiations
// clang-format off

template std::vector<ReferenceDescription> browseAll<Server>(Server&, const BrowseDescription&, uint32_t);
template std::vector<ReferenceDescription> browseAll<Client>(Client&, const BrowseDescription&, uint32_t);

template BrowsePathResult browseSimplifiedBrowsePath<Server>(Server&, const NodeId&, Span<const QualifiedName>);
template BrowsePathResult browseSimplifiedBrowsePath<Client>(Client&, const NodeId&, Span<const QualifiedName>);

// clang-format on

}  // namespace opcua::services
