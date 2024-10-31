#include "open62541pp/services/view.hpp"

#include <cstddef>  // size_t
#include <iterator>  // make_move_iterator

#include "open62541pp/client.hpp"
#include "open62541pp/server.hpp"
#include "open62541pp/services/detail/client_service.hpp"
#include "open62541pp/types.hpp"

namespace opcua::services {

BrowseResponse browse(Client& connection, const BrowseRequest& request) noexcept {
    return UA_Client_Service_browse(connection.handle(), request);
}

template <>
BrowseResult browse<Server>(
    Server& connection, const BrowseDescription& bd, uint32_t maxReferences
) noexcept {
    return UA_Server_browse(connection.handle(), maxReferences, bd.handle());
}

template <>
BrowseResult browse<Client>(
    Client& connection, const BrowseDescription& bd, uint32_t maxReferences
) noexcept {
    const auto request = detail::createBrowseRequest(bd, maxReferences);
    auto response = browse(connection, asWrapper<BrowseRequest>(request));
    return detail::wrapSingleResultWithStatus<BrowseResult>(response);
}

BrowseNextResponse browseNext(Client& connection, const BrowseNextRequest& request) noexcept {
    return UA_Client_Service_browseNext(connection.handle(), request);
}

template <>
BrowseResult browseNext<Server>(
    Server& connection, bool releaseContinuationPoint, const ByteString& continuationPoint
) noexcept {
    return UA_Server_browseNext(
        connection.handle(), releaseContinuationPoint, continuationPoint.handle()
    );
}

template <>
BrowseResult browseNext<Client>(
    Client& connection, bool releaseContinuationPoint, const ByteString& continuationPoint
) noexcept {
    const auto request = detail::createBrowseNextRequest(
        releaseContinuationPoint, continuationPoint
    );
    auto response = browseNext(connection, asWrapper<BrowseNextRequest>(request));
    return detail::wrapSingleResultWithStatus<BrowseResult>(response);
}

TranslateBrowsePathsToNodeIdsResponse translateBrowsePathsToNodeIds(
    Client& connection, const TranslateBrowsePathsToNodeIdsRequest& request
) noexcept {
    return UA_Client_Service_translateBrowsePathsToNodeIds(connection.handle(), request);
}

template <>
BrowsePathResult translateBrowsePathToNodeIds<Server>(
    Server& connection, const BrowsePath& browsePath
) noexcept {
    return UA_Server_translateBrowsePathToNodeIds(connection.handle(), browsePath.handle());
}

template <>
BrowsePathResult translateBrowsePathToNodeIds<Client>(
    Client& connection, const BrowsePath& browsePath
) noexcept {
    const auto request = detail::createTranslateBrowsePathsToNodeIdsRequest(browsePath);
    auto response = translateBrowsePathsToNodeIds(
        connection, asWrapper<TranslateBrowsePathsToNodeIdsRequest>(request)
    );
    return detail::wrapSingleResultWithStatus<BrowsePathResult>(response);
}

RegisterNodesResponse registerNodes(
    Client& connection, const RegisterNodesRequest& request
) noexcept {
    return UA_Client_Service_registerNodes(connection.handle(), request);
}

UnregisterNodesResponse unregisterNodes(
    Client& connection, const UnregisterNodesRequest& request
) noexcept {
    return UA_Client_Service_unregisterNodes(connection.handle(), request);
}

template <typename T>
Result<std::vector<ReferenceDescription>> browseAll(
    T& connection, const BrowseDescription& bd, uint32_t maxReferences
) {
    std::vector<ReferenceDescription> refs;
    auto append = [&](Span<ReferenceDescription> refsNew) {
        refs.insert(
            refs.end(),
            std::make_move_iterator(refsNew.begin()),
            std::make_move_iterator(refsNew.end())
        );
    };
    BrowseResult result = browse(connection, bd, maxReferences);
    append(result.getReferences());
    while (!result.getContinuationPoint().empty()) {
        result = browseNext(connection, false, result.getContinuationPoint());
        append(result.getReferences());
    }
    if (result.getStatusCode().isBad()) {
        return BadResult(result.getStatusCode());
    }
    if ((maxReferences > 0) && (refs.size() > maxReferences)) {
        refs.resize(maxReferences);
    }
    return refs;
}

template Result<std::vector<ReferenceDescription>> browseAll<Client>(
    Client&, const BrowseDescription&, uint32_t
);
template Result<std::vector<ReferenceDescription>> browseAll<Server>(
    Server&, const BrowseDescription&, uint32_t
);

Result<std::vector<ExpandedNodeId>> browseRecursive(
    Server& connection, const BrowseDescription& bd
) {
    size_t arraySize{};
    UA_ExpandedNodeId* array{};
    const StatusCode status = UA_Server_browseRecursive(
        connection.handle(), bd.handle(), &arraySize, &array
    );
    std::vector<ExpandedNodeId> result(
        std::make_move_iterator(array),
        std::make_move_iterator(array + arraySize)  // NOLINT
    );
    UA_free(array);  // NOLINT
    if (status.isBad()) {
        return BadResult(status);
    }
    return result;
}

}  // namespace opcua::services
