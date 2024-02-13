#include "open62541pp/services/View.h"

#include <cstddef>  // size_t

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/NodeIds.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/services/detail/ClientService.h"
#include "open62541pp/services/detail/ResponseHandling.h"
#include "open62541pp/types/Builtin.h"

#include "../open62541_impl.h"

namespace opcua::services {

BrowseResponse browse(Client& connection, const BrowseRequest& request) {
    return browseAsync(connection, request, detail::SyncOperation{});
}

template <>
BrowseResult browse<Server>(
    Server& connection, const BrowseDescription& bd, uint32_t maxReferences
) {
    return UA_Server_browse(connection.handle(), maxReferences, bd.handle());
}

template <>
BrowseResult browse<Client>(
    Client& connection, const BrowseDescription& bd, uint32_t maxReferences
) {
    return browseAsync(connection, bd, maxReferences, detail::SyncOperation{});
}

BrowseNextResponse browseNext(Client& connection, const BrowseNextRequest& request) {
    return browseNextAsync(connection, request, detail::SyncOperation{});
}

template <>
BrowseResult browseNext<Server>(
    Server& connection, bool releaseContinuationPoint, const ByteString& continuationPoint
) {
    return UA_Server_browseNext(
        connection.handle(), releaseContinuationPoint, continuationPoint.handle()
    );
}

template <>
BrowseResult browseNext<Client>(
    Client& connection, bool releaseContinuationPoint, const ByteString& continuationPoint
) {
    return browseNextAsync(
        connection, releaseContinuationPoint, continuationPoint, detail::SyncOperation{}
    );
}

TranslateBrowsePathsToNodeIdsResponse translateBrowsePathsToNodeIds(
    Client& connection, const TranslateBrowsePathsToNodeIdsRequest& request
) {
    return translateBrowsePathsToNodeIdsAsync(connection, request, detail::SyncOperation{});
}

template <>
BrowsePathResult translateBrowsePathToNodeIds<Server>(
    Server& connection, const BrowsePath& browsePath
) {
    return UA_Server_translateBrowsePathToNodeIds(connection.handle(), browsePath.handle());
}

template <>
BrowsePathResult translateBrowsePathToNodeIds<Client>(
    Client& connection, const BrowsePath& browsePath
) {
    return translateBrowsePathToNodeIdsAsync(connection, browsePath, detail::SyncOperation{});
}

RegisterNodesResponse registerNodes(Client& connection, const RegisterNodesRequest& request) {
    return registerNodesAsync(connection, request, detail::SyncOperation{});
}

UnregisterNodesResponse unregisterNodes(Client& connection, const UnregisterNodesRequest& request) {
    return unregisterNodesAsync(connection, request, detail::SyncOperation{});
}

std::vector<ExpandedNodeId> browseRecursive(Server& connection, const BrowseDescription& bd) {
    size_t arraySize{};
    UA_ExpandedNodeId* array{};
    const auto status = UA_Server_browseRecursive(
        connection.handle(), bd.handle(), &arraySize, &array
    );
    std::vector<ExpandedNodeId> result(
        std::make_move_iterator(array),
        std::make_move_iterator(array + arraySize)  // NOLINT
    );
    UA_free(array);  // NOLINT
    throwIfBad(status);
    return result;
}

}  // namespace opcua::services
