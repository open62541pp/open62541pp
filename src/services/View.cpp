#include "open62541pp/services/View.h"

#include <algorithm>  // transform
#include <cstddef>
#include <utility>  // exchange

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/NodeIds.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/ClientService.h"
#include "open62541pp/types/Builtin.h"

#include "../open62541_impl.h"

namespace opcua::services {

BrowseResponse browse(Client& client, const BrowseRequest& request) {
    return detail::sendRequest<UA_BrowseRequest, UA_BrowseResponse>(
        client, request, detail::MoveResponse{}, detail::SyncOperation{}
    );
}

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
    request.nodesToBrowse = const_cast<UA_BrowseDescription*>(bd.handle());  // NOLINT

    auto response = browse(client, asWrapper<BrowseRequest>(request));
    return std::move(detail::getSingleResultFromResponse(response));
}

BrowseNextResponse browseNext(Client& client, const BrowseNextRequest& request) {
    return detail::sendRequest<UA_BrowseNextRequest, UA_BrowseNextResponse>(
        client, request, detail::MoveResponse{}, detail::SyncOperation{}
    );
}

template <>
BrowseResult browseNext<Server>(
    Server& server, bool releaseContinuationPoint, const ByteString& continuationPoint
) {
    BrowseResult result = UA_Server_browseNext(
        server.handle(), releaseContinuationPoint, continuationPoint.handle()
    );
    detail::throwOnBadStatus(result->statusCode);  // TODO: remove?
    return result;
}

template <>
BrowseResult browseNext<Client>(
    Client& client, bool releaseContinuationPoint, const ByteString& continuationPoint
) {
    UA_BrowseNextRequest request{};
    request.releaseContinuationPoints = releaseContinuationPoint;
    request.continuationPointsSize = 1;
    request.continuationPoints = const_cast<UA_ByteString*>(continuationPoint.handle());  // NOLINT

    auto response = browseNext(client, asWrapper<BrowseNextRequest>(request));
    return std::move(detail::getSingleResultFromResponse(response));
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
    size_t arraySize{};
    UA_ExpandedNodeId* array{};
    const auto status = UA_Server_browseRecursive(server.handle(), bd.handle(), &arraySize, &array);
    std::vector<ExpandedNodeId> result(
        std::make_move_iterator(array),
        std::make_move_iterator(array + arraySize)  // NOLINT
    );
    UA_free(array);  // NOLINT
    detail::throwOnBadStatus(status);
    return result;
}

TranslateBrowsePathsToNodeIdsResponse translateBrowsePathsToNodeIds(
    Client& client, const TranslateBrowsePathsToNodeIdsRequest& request
) {
    return detail::sendRequest<
        UA_TranslateBrowsePathsToNodeIdsRequest,
        UA_TranslateBrowsePathsToNodeIdsResponse>(
        client, request, detail::MoveResponse{}, detail::SyncOperation{}
    );
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
    request.browsePaths = const_cast<UA_BrowsePath*>(browsePath.handle());  // NOLINT

    auto response = translateBrowsePathsToNodeIds(
        client, asWrapper<TranslateBrowsePathsToNodeIdsRequest>(request)
    );
    return std::move(detail::getSingleResultFromResponse(response));
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

RegisterNodesResponse registerNodes(Client& client, const RegisterNodesRequest& request) {
    return detail::sendRequest<UA_RegisterNodesRequest, UA_RegisterNodesResponse>(
        client, request, detail::MoveResponse{}, detail::SyncOperation{}
    );
}

UnregisterNodesResponse unregisterNodes(Client& client, const UnregisterNodesRequest& request) {
    return detail::sendRequest<UA_UnregisterNodesRequest, UA_UnregisterNodesResponse>(
        client, request, detail::MoveResponse{}, detail::SyncOperation{}
    );
}

// explicit template instantiations
// clang-format off

template std::vector<ReferenceDescription> browseAll<Server>(Server&, const BrowseDescription&, uint32_t);
template std::vector<ReferenceDescription> browseAll<Client>(Client&, const BrowseDescription&, uint32_t);

template BrowsePathResult browseSimplifiedBrowsePath<Server>(Server&, const NodeId&, Span<const QualifiedName>);
template BrowsePathResult browseSimplifiedBrowsePath<Client>(Client&, const NodeId&, Span<const QualifiedName>);

// clang-format on

}  // namespace opcua::services
