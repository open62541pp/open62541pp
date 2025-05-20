#include "open62541pp/services/view.hpp"

#include <cstddef>  // size_t

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

}  // namespace opcua::services
