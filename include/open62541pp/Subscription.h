#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "open62541pp/Common.h"
#include "open62541pp/MonitoredItem.h"
#include "open62541pp/services/MonitoredItem.h"
#include "open62541pp/services/Subscription.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

namespace opcua {

// forward declarations
class Client;
class Server;

using SubscriptionParameters = services::SubscriptionParameters;
using MonitoringParameters = services::MonitoringParameters;

/// Data change notification callback.
template <typename T>
using DataChangeCallback =
    std::function<void(const MonitoredItem<T>& item, const DataValue& value)>;

/// Event notification callback.
template <typename T>
using EventCallback =
    std::function<void(const MonitoredItem<T>& item, const std::vector<Variant>& eventFields)>;

/**
 * High-level subscription class with template specializations for Server and Client.
 *
 * @see Subscription<Server>
 * @see Subscription<Client>
 *
 * Use the free functions in the `services` namespace for more advanced usage:
 * - @ref Subscription
 */
template <typename ServerOrClient>
class Subscription;

/* ------------------------------------------- Server ------------------------------------------- */

/**
 * High-level subscription class for servers.
 *
 * Servers don't use the subscription mechanism of OPC UA to transport notifications of data changes
 * and events. Instead MonitoredItems are registered locally. Notifications are then forwarded to
 * user-defined callbacks instead of a remote client.
 */
template <>
class Subscription<Server> {
public:
    explicit Subscription(Server& server) noexcept;

    /// Get the server instance.
    Server& getConnection() noexcept;
    /// Get the server instance.
    const Server& getConnection() const noexcept;

    /// Get all local monitored items.
    std::vector<MonitoredItem<Server>> getMonitoredItems();

    /// Create a local monitored item for data change notifications.
    /// @copydetails services::MonitoringParameters
    MonitoredItem<Server> subscribeDataChange(
        const NodeId& id,
        AttributeId attribute,
        MonitoringMode monitoringMode,
        MonitoringParameters& parameters,
        DataChangeCallback<Server> onDataChange
    );

private:
    Server& server_;
};

bool operator==(const Subscription<Server>& left, const Subscription<Server>& right) noexcept;
bool operator!=(const Subscription<Server>& left, const Subscription<Server>& right) noexcept;

/* ------------------------------------------- Client ------------------------------------------- */

/**
 * High-level subscription class for clients.
 */
template <>
class Subscription<Client> {
public:
    Subscription(Client& client, uint32_t subscriptionId) noexcept;

    /// Get the client instance.
    Client& getConnection() noexcept;
    /// Get the client instance.
    const Client& getConnection() const noexcept;

    /// Get the server-assigned identifier of this subscription.
    uint32_t getSubscriptionId() const noexcept;

    /// Get all monitored items of this subscription.
    std::vector<MonitoredItem<Client>> getMonitoredItems();

    /// Modify this subscription.
    /// @see services::modifySubscription
    void setSubscriptionParameters(SubscriptionParameters& parameters);

    /// Enable/disable publishing of notification messages.
    /// @see services::setPublishingMode
    void setPublishingMode(bool publishing);

    /// Create a monitored item for data change notifications.
    /// @copydetails services::MonitoringParameters
    MonitoredItem<Client> subscribeDataChange(
        const NodeId& id,
        AttributeId attribute,
        MonitoringMode monitoringMode,
        MonitoringParameters& parameters,
        DataChangeCallback<Client> onDataChange
    );

    /// Create a monitored item for data change notifications.
    /// @copydetails services::MonitoringParameters
    MonitoredItem<Client> subscribeEvent(
        const NodeId& id,
        MonitoringMode monitoringMode,
        MonitoringParameters& parameters,
        EventCallback<Client> onEvent
    );

    /// Delete this subscription.
    /// @see services::deleteSubscription
    void deleteSubscription();

private:
    Client& client_;
    uint32_t subscriptionId_;
};

bool operator==(Subscription<Client>& left, Subscription<Client>& right) noexcept;
bool operator!=(Subscription<Client>& left, Subscription<Client>& right) noexcept;

}  // namespace opcua
