#include "open62541pp/services/view.hpp"

#include <cstddef>  // size_t

#include "open62541pp/client.hpp"
#include "open62541pp/detail/open62541/server.h"
#include "open62541pp/server.hpp"
#include "open62541pp/services/detail/client_services.hpp"
#include "open62541pp/types.hpp"

namespace opcua::services {

BrowseResponse browse(Client& connection, const BrowseRequest& request) noexcept {
    return browseAsync(connection, request, detail::SyncOperation{});
}

template <>
Result<BrowseResult> browse<Server>(
    Server& connection, const BrowseDescription& bd, uint32_t maxReferences
) noexcept {
    return {UA_Server_browse(connection.handle(), maxReferences, bd.handle())};
}

template <>
Result<BrowseResult> browse<Client>(
    Client& connection, const BrowseDescription& bd, uint32_t maxReferences
) noexcept {
    return browseAsync(connection, bd, maxReferences, detail::SyncOperation{});
}

BrowseNextResponse browseNext(Client& connection, const BrowseNextRequest& request) noexcept {
    return browseNextAsync(connection, request, detail::SyncOperation{});
}

template <>
Result<BrowseResult> browseNext<Server>(
    Server& connection, bool releaseContinuationPoint, const ByteString& continuationPoint
) noexcept {
    return {UA_Server_browseNext(
        connection.handle(), releaseContinuationPoint, continuationPoint.handle()
    )};
}

template <>
Result<BrowseResult> browseNext<Client>(
    Client& connection, bool releaseContinuationPoint, const ByteString& continuationPoint
) noexcept {
    return browseNextAsync(
        connection, releaseContinuationPoint, continuationPoint, detail::SyncOperation{}
    );
}

TranslateBrowsePathsToNodeIdsResponse translateBrowsePathsToNodeIds(
    Client& connection, const TranslateBrowsePathsToNodeIdsRequest& request
) noexcept {
    return translateBrowsePathsToNodeIdsAsync(connection, request, detail::SyncOperation{});
}

template <>
Result<BrowsePathResult> translateBrowsePathToNodeIds<Server>(
    Server& connection, const BrowsePath& browsePath
) noexcept {
    return {UA_Server_translateBrowsePathToNodeIds(connection.handle(), browsePath.handle())};
}

template <>
Result<BrowsePathResult> translateBrowsePathToNodeIds<Client>(
    Client& connection, const BrowsePath& browsePath
) noexcept {
    return translateBrowsePathToNodeIdsAsync(connection, browsePath, detail::SyncOperation{});
}

RegisterNodesResponse registerNodes(
    Client& connection, const RegisterNodesRequest& request
) noexcept {
    return registerNodesAsync(connection, request, detail::SyncOperation{});
}

UnregisterNodesResponse unregisterNodes(
    Client& connection, const UnregisterNodesRequest& request
) noexcept {
    return unregisterNodesAsync(connection, request, detail::SyncOperation{});
}

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
