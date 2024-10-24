#pragma once

#include <cstdint>
#include <iterator>  // make_move_iterator
#include <utility>
#include <vector>

#include "open62541pp/async.hpp"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/result.hpp"
#include "open62541pp/services/detail/client_services.hpp"
#include "open62541pp/services/detail/request_handling.hpp"
#include "open62541pp/services/detail/response_handling.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/types_composed.hpp"

namespace opcua {
class Client;
class Server;
}  // namespace opcua

namespace opcua::services {

/**
 * @defgroup View View service set
 * Browse the address space / view created by a server.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8
 * @ingroup Services
 * @{
 */

/**
 * @defgroup Browse Browse service
 * Discover references of nodes.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.2
 * @{
 */

/**
 * Discover the references of one or more nodes (client only).
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.2
 * @param connection Instance of type Client
 * @param request Browse request
 */
BrowseResponse browse(Client& connection, const BrowseRequest& request) noexcept;

/**
 * Asynchronously discover the references of one or more nodes (client only).
 * @copydetails browse
 * @param token @completiontoken{void(BrowseResponse&)}
 * @return @asyncresult{BrowseResponse}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto browseAsync(
    Client& connection,
    const BrowseRequest& request,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<UA_BrowseRequest, UA_BrowseResponse>(
        connection, request, detail::Wrap<BrowseResponse>{}, std::forward<CompletionToken>(token)
    );
}

/**
 * Discover the references of a specified node.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.2
 * @param connection Instance of type Server or Client
 * @param bd Browse description
 * @param maxReferences The maximum number of references to return (0 if no limit)
 */
template <typename T>
Result<BrowseResult> browse(
    T& connection, const BrowseDescription& bd, uint32_t maxReferences = 0
) noexcept;

/**
 * Asynchronously discover the references of a specified node.
 * @copydetails browse(T&, const BrowseDescription&, uint32_t)
 * @param token @completiontoken{void(Result<BrowseResult>&)}
 * @return @asyncresult{Result<BrowseResult>}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto browseAsync(
    Client& connection,
    const BrowseDescription& bd,
    uint32_t maxReferences = 0,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<UA_BrowseRequest, UA_BrowseResponse>(
        connection,
        detail::createBrowseRequest(bd, maxReferences),
        [](UA_BrowseResponse& response) {
            return detail::getSingleResult(response).transform(detail::Wrap<BrowseResult>{});
        },
        std::forward<CompletionToken>(token)
    );
}

/**
 * @}
 * @defgroup BrowseNext BrowseNext service
 * Request the next set of a Browse or BrowseNext response information that is too large to be sent
 * in a single response. Discover references of nodes.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.3
 * @{
 */

/**
 * Request the next sets of @ref browse / @ref browseNext responses (client only).
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.3
 * @param connection Instance of type Client
 * @param request Browse request
 */
BrowseNextResponse browseNext(Client& connection, const BrowseNextRequest& request) noexcept;

/**
 * Asynchronously request the next sets of @ref browse / @ref browseNext responses (client only).
 * @copydetails browseNext
 * @param token @completiontoken{void(BrowseNextResponse&)}
 * @return @asyncresult{BrowseNextResponse}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto browseNextAsync(
    Client& connection,
    const BrowseNextRequest& request,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<UA_BrowseNextRequest, UA_BrowseNextResponse>(
        connection,
        request,
        detail::Wrap<BrowseNextResponse>{},
        std::forward<CompletionToken>(token)
    );
}

/**
 * Request the next set of a @ref browse or @ref browseNext response.
 * The response might get split up if the information is too large to be sent in a single response.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.3
 * @param connection Instance of type Server or Client
 * @param releaseContinuationPoint Free resources in server if `true`, get next result if `false`
 * @param continuationPoint Continuation point from a preview browse/browseNext request
 */
template <typename T>
Result<BrowseResult> browseNext(
    T& connection, bool releaseContinuationPoint, const ByteString& continuationPoint
) noexcept;

/**
 * Asynchronously request the next set of a @ref browse or @ref browseNext response.
 * @copydetails browseNext(T&, bool, const ByteString&)
 * @param token @completiontoken{void(Result<BrowseResult>&)}
 * @return @asyncresult{Result<BrowseResult>}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto browseNextAsync(
    Client& connection,
    bool releaseContinuationPoint,
    const ByteString& continuationPoint,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<UA_BrowseNextRequest, UA_BrowseNextResponse>(
        connection,
        detail::createBrowseNextRequest(releaseContinuationPoint, continuationPoint),
        [](UA_BrowseNextResponse& response) {
            return detail::getSingleResult(response).transform(detail::Wrap<BrowseResult>{});
        },
        std::forward<CompletionToken>(token)
    );
}

/**
 * @}
 * @defgroup TranslateBrowsePathsToNodeIds TranslateBrowsePathsToNodeIds service
 * Request that the server translates browse paths to node ids.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.4
 * @{
 */

/**
 * Translate browse paths to NodeIds (client only).
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.4
 * @param connection Instance of type Client
 * @param request Request
 */
TranslateBrowsePathsToNodeIdsResponse translateBrowsePathsToNodeIds(
    Client& connection, const TranslateBrowsePathsToNodeIdsRequest& request
) noexcept;

/**
 * Asynchronously translate browse paths to NodeIds (client only).
 * @copydetails translateBrowsePathsToNodeIds
 * @param token @completiontoken{void(TranslateBrowsePathsToNodeIdsResponse&)}
 * @return @asyncresult{TranslateBrowsePathsToNodeIdsResponse}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto translateBrowsePathsToNodeIdsAsync(
    Client& connection,
    const TranslateBrowsePathsToNodeIdsRequest& request,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<
        UA_TranslateBrowsePathsToNodeIdsRequest,
        UA_TranslateBrowsePathsToNodeIdsResponse>(
        connection,
        request,
        detail::Wrap<TranslateBrowsePathsToNodeIdsResponse>{},
        std::forward<CompletionToken>(token)
    );
}

/**
 * Translate a browse path to NodeIds.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.4
 * @param connection Instance of type Server or Client
 * @param browsePath Browse path (starting node & relative path)
 */
template <typename T>
Result<BrowsePathResult> translateBrowsePathToNodeIds(
    T& connection, const BrowsePath& browsePath
) noexcept;

/**
 * Asynchronously translate a browse path to NodeIds.
 * @copydetails translateBrowsePathToNodeIds
 * @param token @completiontoken{void(Result<BrowsePathResult>&)}
 * @return @asyncresult{Result<BrowsePathResult>}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto translateBrowsePathToNodeIdsAsync(
    Client& connection,
    const BrowsePath& browsePath,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<
        UA_TranslateBrowsePathsToNodeIdsRequest,
        UA_TranslateBrowsePathsToNodeIdsResponse>(
        connection,
        detail::createTranslateBrowsePathsToNodeIdsRequest(browsePath),
        [](UA_TranslateBrowsePathsToNodeIdsResponse& response) {
            return detail::getSingleResult(response).transform(detail::Wrap<BrowsePathResult>{});
        },
        std::forward<CompletionToken>(token)
    );
}

/**
 * A simplified version of @ref translateBrowsePathToNodeIds.
 *
 * The relative path is specified using browse names instead of the RelativePath structure.
 * The list of browse names is equivalent to a RelativePath that specifies forward references which
 * are subtypes of the HierarchicalReferences ReferenceTypeId.
 *
 * @param connection Instance of type Server or Client
 * @param origin Starting node of the browse path
 * @param browsePath Browse path as a list of browse names
 */
template <typename T>
inline Result<BrowsePathResult> browseSimplifiedBrowsePath(
    T& connection, const NodeId& origin, Span<const QualifiedName> browsePath
) {
    return translateBrowsePathToNodeIds(connection, detail::createBrowsePath(origin, browsePath));
}

/**
 * A simplified version of @ref translateBrowsePathToNodeIdsAsync.
 * @copydetails browseSimplifiedBrowsePath
 * @param token @completiontoken{void(Result<BrowsePathResult>&)}
 * @return @asyncresult{Result<BrowsePathResult>}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto browseSimplifiedBrowsePathAsync(
    Client& connection,
    const NodeId& origin,
    Span<const QualifiedName> browsePath,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return translateBrowsePathToNodeIdsAsync(
        connection,
        detail::createBrowsePath(origin, browsePath),
        std::forward<CompletionToken>(token)
    );
}

/**
 * @}
 * @defgroup RegisterNodes RegisterNodes service
 * Register nodes for efficient access operations.
 * Clients shall unregister unneeded nodes immediately to free up resources.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.5
 * @{
 */

/**
 * Register nodes for efficient access operations (client only).
 * @param connection Instance of type Client
 * @param request Request
 */
RegisterNodesResponse registerNodes(
    Client& connection, const RegisterNodesRequest& request
) noexcept;

/**
 * Asynchronously register nodes for efficient access operations (client only).
 * @copydetails registerNodes
 * @param token @completiontoken{void(RegisterNodesResponse&)}
 * @return @asyncresult{RegisterNodesResponse}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto registerNodesAsync(
    Client& connection,
    const RegisterNodesRequest& request,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<UA_RegisterNodesRequest, UA_RegisterNodesResponse>(
        connection,
        request,
        detail::Wrap<RegisterNodesResponse>{},
        std::forward<CompletionToken>(token)
    );
}

/**
 * @}
 * @defgroup UnregisterNodes UnregisterNodes service
 * Unregister nodes that have been obtained via the RegisterNodes service.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.6
 * @{
 */

/**
 * Unregister nodes (client only).
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.6
 * @param connection Instance of type Client
 * @param request Request
 */
UnregisterNodesResponse unregisterNodes(
    Client& connection, const UnregisterNodesRequest& request
) noexcept;

/**
 * Asynchronously unregister nodes (client only).
 * @copydetails unregisterNodes
 * @param token @completiontoken{void(UnregisterNodesResponse&)}
 * @return @asyncresult{UnregisterNodesResponse}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto unregisterNodesAsync(
    Client& connection,
    const UnregisterNodesRequest& request,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<UA_UnregisterNodesRequest, UA_UnregisterNodesResponse>(
        connection,
        request,
        detail::Wrap<UnregisterNodesResponse>{},
        std::forward<CompletionToken>(token)
    );
}

/**
 * @}
 */

/* ----------------------------------- Non-standard functions ----------------------------------- */

/**
 * Discover all the references of a specified node (without calling @ref browseNext).
 * @copydetails browse(T&, const BrowseDescription&, uint32_t)
 * @ingroup Browse
 */
template <typename T>
Result<std::vector<ReferenceDescription>> browseAll(
    T& connection, const BrowseDescription& bd, uint32_t maxReferences = 0
) {
    std::vector<ReferenceDescription> refs;
    auto append = [&](Span<ReferenceDescription> refsNew) {
        refs.insert(
            refs.end(),
            std::make_move_iterator(refsNew.begin()),
            std::make_move_iterator(refsNew.end())
        );
    };
    Result<BrowseResult> result = browse(connection, bd, maxReferences);
    if (!result) {
        return BadResult(result.code());
    }
    append(result->getReferences());
    while (!result.value().getContinuationPoint().empty()) {
        const bool release = (refs.size() >= maxReferences);
        result = browseNext(connection, release, result->getContinuationPoint());
        if (!result) {
            return BadResult(result.code());
        }
        append(result->getReferences());
    }
    if ((maxReferences > 0) && (refs.size() > maxReferences)) {
        refs.resize(maxReferences);
    }
    return refs;
}

/**
 * Discover child nodes recursively (non-standard).
 *
 * Possible loops (that can occur for non-hierarchical references) are handled internally. Every
 * node is added at most once to the results array. Nodes are only added if they match the
 * `nodeClassMask` in the BrowseDescription. However, child nodes are still recursed into if the
 * NodeClass does not match. So it is possible, for example, to get all VariableNodes below a
 * certain ObjectNode, with additional objects in the hierarchy below.
 *
 * @note No implementation for `Client`.
 *
 * @param connection Instance of type Server
 * @param bd Browse description
 * @see UA_Server_browseRecursive
 * @ingroup Browse
 */
Result<std::vector<ExpandedNodeId>> browseRecursive(
    Server& connection, const BrowseDescription& bd
);

/**
 * @}
 */

}  // namespace opcua::services
