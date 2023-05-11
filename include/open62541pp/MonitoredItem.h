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
 * High-level monitored item class with template specializations for Server and Client.
 *
 * @see MonitoredItem<Server>
 * @see MonitoredItem<Client>
 *
 * Use the free functions in the `services` namespace for more advanced usage:
 * - @ref MonitoredItem
 */
template <typename ServerOrClient>
class MonitoredItem;

/* ------------------------------------------- Server ------------------------------------------- */

/**
 * High-level monitored item class for servers.
 */
template <>
class MonitoredItem<Server> {
public:
    MonitoredItem(Server& server, uint32_t monitoredItemId) noexcept;

    /// Get the server instance.
    Server& getConnection() noexcept;
    /// Get the server instance.
    const Server& getConnection() const noexcept;

    /// Get the server-assigned identifier of this monitored item.
    uint32_t getMonitoredItemId() const noexcept;

    /// Get the underlying subscription.
    Subscription<Server> getSubscription() const;

    /// Get the monitored NodeId.
    const NodeId& getNodeId() const;

    /// Get the monitored AttributeId.
    AttributeId getAttributeId() const;

    /// Delete this monitored item.
    /// @see services::deleteMonitoredItem(Server&, uint32_t)
    void deleteMonitoredItem();

private:
    Server& server_;
    uint32_t monitoredItemId_;
};

bool operator==(const MonitoredItem<Server>& left, const MonitoredItem<Server>& right) noexcept;
bool operator!=(const MonitoredItem<Server>& left, const MonitoredItem<Server>& right) noexcept;

/* ------------------------------------------- Client ------------------------------------------- */

/**
 * High-level monitored item class for clients.
 */
template <>
class MonitoredItem<Client> {
public:
    MonitoredItem(Client& client, uint32_t subscriptionId, uint32_t monitoredItemId) noexcept;

    /// Get the client instance.
    Client& getConnection() noexcept;
    /// Get the client instance.
    const Client& getConnection() const noexcept;

    /// Get the server-assigned identifier of the underlying subscription.
    uint32_t getSubscriptionId() const noexcept;
    /// Get the server-assigned identifier of this monitored item.
    uint32_t getMonitoredItemId() const noexcept;

    /// Get the underlying subscription.
    Subscription<Client> getSubscription() const;

    /// Get the monitored NodeId.
    const NodeId& getNodeId() const;

    /// Get the monitored AttributeId.
    AttributeId getAttributeId() const;

    /// Modify this monitored item.
    /// @see services::modifyMonitoredItem
    void setMonitoringParameters(MonitoringParameters& parameters);

    /// Set the monitoring mode of this monitored item.
    /// @see services::setMonitoringMode
    void setMonitoringMode(MonitoringMode monitoringMode);

    /// Delete this monitored item.
    /// @see services::deleteMonitoredItem(Client&, uint32_t, uint32_t)
    void deleteMonitoredItem();

private:
    Client& client_;
    uint32_t subscriptionId_;
    uint32_t monitoredItemId_;
};

bool operator==(const MonitoredItem<Client>& left, const MonitoredItem<Client>& right) noexcept;
bool operator!=(const MonitoredItem<Client>& left, const MonitoredItem<Client>& right) noexcept;

}  // namespace opcua
