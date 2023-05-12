#pragma once

#include <cstdint>

#include "open62541pp/Common.h"
#include "open62541pp/services/MonitoredItem.h"
#include "open62541pp/types/NodeId.h"

namespace opcua {

// forward declarations
class Client;
class Server;
template <typename ServerOrClient>
class Subscription;

using MonitoringParameters = services::MonitoringParameters;

/**
 * High-level monitored item class.
 *
 * @note Not all methods are available and implemented for servers.
 *
 * Use the free functions in the `services` namespace for more advanced usage:
 * - @ref MonitoredItem
 */
template <typename ServerOrClient>
class MonitoredItem {
public:
    /// Wrap an existing monitored item.
    /// The `subscriptionId` is ignored and set to `0U` for local monitored items within servers.
    MonitoredItem(
        ServerOrClient& connection, uint32_t subscriptionId, uint32_t monitoredItemId
    ) noexcept;

    /// Get the server/client instance.
    ServerOrClient& getConnection() noexcept;
    /// Get the server/client instance.
    const ServerOrClient& getConnection() const noexcept;

    /// Get the server-assigned identifier of the underlying subscription.
    uint32_t getSubscriptionId() const noexcept;
    /// Get the server-assigned identifier of this monitored item.
    uint32_t getMonitoredItemId() const noexcept;

    /// Get the underlying subscription.
    Subscription<ServerOrClient> getSubscription() const;

    /// Get the monitored NodeId.
    const NodeId& getNodeId() const;

    /// Get the monitored AttributeId.
    AttributeId getAttributeId() const;

    /// Modify this monitored item.
    /// @note Not implemented for Server.
    /// @see services::modifyMonitoredItem
    void setMonitoringParameters(MonitoringParameters& parameters);

    /// Set the monitoring mode of this monitored item.
    /// @note Not implemented for Server.
    /// @see services::setMonitoringMode
    void setMonitoringMode(MonitoringMode monitoringMode);

    /// Delete this monitored item.
    /// @see services::deleteMonitoredItem
    void deleteMonitoredItem();

private:
    ServerOrClient& connection_;
    uint32_t subscriptionId_{0U};
    uint32_t monitoredItemId_{0U};
};

/* ---------------------------------------------------------------------------------------------- */

template <typename T>
inline bool operator==(const MonitoredItem<T>& left, const MonitoredItem<T>& right) noexcept {
    return (left.getConnection() == right.getConnection()) &&
           (left.getSubscriptionId() == right.getSubscriptionId()) &&
           (left.getMonitoredItemId() == right.getMonitoredItemId());
}

template <typename T>
inline bool operator!=(const MonitoredItem<T>& left, const MonitoredItem<T>& right) noexcept {
    return !(left == right);
}

}  // namespace opcua
