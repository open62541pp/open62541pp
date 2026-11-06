#pragma once

#include <cstddef>  // size_t
#include <cstdint>  // int32_t

#include "open62541pp/config.hpp"

#if UAPP_HAS_HISTORIZING

namespace opcua {

class NodeId;
class Server;
class StatusCode;

/**
 * Strategy to store the values of a historized node.
 * @see UA_HistorizingUpdateStrategy
 */
enum class HistorizingUpdateStrategy : int32_t {
    /// Values are stored by the user, the server doesn't store values on its own.
    User = 0,
    /// Values are stored when the node's value is set, e.g. with the write service.
    ValueSet = 1,
    /// The node's value is read periodically, see HistorizingSettings::pollingInterval.
    /// Mainly relevant for data source nodes, which are not updated with the write service.
    Poll = 2,
};

/**
 * Settings to historize a node with the in-memory history data backend.
 */
struct HistorizingSettings {
    /// Maximum number of values returned by the server in one response.
    /// Continuation points are used if more values are available.
    size_t maxResponseSize = 100;
    /// Strategy to store the values of the node.
    HistorizingUpdateStrategy updateStrategy = HistorizingUpdateStrategy::ValueSet;
    /// Polling interval in milliseconds for HistorizingUpdateStrategy::Poll.
    size_t pollingInterval = 1000;
    /// Number of values the backend stores for the node before reallocating.
    size_t capacity = 100;
};

/**
 * Enable historical data access with open62541's default history database.
 *
 * The values are stored in memory. Nodes are historized with @ref historizeNode.
 * Subsequent calls have no effect.
 *
 * @param server Server instance
 * @param capacity Number of nodes the history database stores before reallocating
 *
 * @note Must be called before the server is started.
 * @see https://www.open62541.org/doc/1.4/tutorial_server_historicaldata.html
 * @relates Server
 */
void useHistoryDatabase(Server& server, size_t capacity = 1);

/**
 * Historize a variable node.
 *
 * Registers the node with the history database and sets its `Historizing` attribute.
 * Clients can read the history of the node if its access level contains
 * @ref AccessLevel::HistoryRead.
 *
 * @param server Server instance
 * @param id Node to historize
 * @param settings Historizing settings of the node
 * @return `BadConfigurationError` if @ref useHistoryDatabase was not called before
 *
 * @relates Server
 */
StatusCode historizeNode(
    Server& server, const NodeId& id, const HistorizingSettings& settings = {}
);

}  // namespace opcua

#endif
