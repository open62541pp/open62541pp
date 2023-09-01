#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "open62541pp/Common.h"
#include "open62541pp/Config.h"
#include "open62541pp/MonitoredItem.h"
#include "open62541pp/services/MonitoredItem.h"
#include "open62541pp/services/Subscription.h"
#include "open62541pp/types/NodeId.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS

namespace opcua {

// forward declarations
class Client;
class DataValue;
class EventFilter;
class Server;
template <typename T>
class Span;
class Variant;

using SubscriptionParameters = services::SubscriptionParameters;
using MonitoringParameters = services::MonitoringParameters;

/// Data change notification callback.
/// @tparam T Server or Client
template <typename T>
using DataChangeCallback =
    std::function<void(const MonitoredItem<T>& item, const DataValue& value)>;

/// Event notification callback.
/// @tparam T Server or Client
template <typename T>
using EventCallback =
    std::function<void(const MonitoredItem<T>& item, Span<const Variant> eventFields)>;

/**
 * High-level subscription class.
 *
 * The API is symmetric for both Server and Client, although servers don't use the subscription
 * mechanism of OPC UA to transport notifications of data changes and events. Instead MonitoredItems
 * are registered locally. Notifications are then forwarded to user-defined callbacks instead of a
 * remote client. The `subscriptionId` for servers is always `0U`.
 *
 * @note Not all methods are available and implemented for servers.
 *
 * Use the free functions in the `services` namespace for more advanced usage:
 * - @ref Subscription
 * - @ref MonitoredItem
 */
template <typename ServerOrClient>
class Subscription {
public:
    /// Wrap an existing subscription.
    /// The `subscriptionId` is ignored and set to `0U` for servers.
    Subscription(ServerOrClient& connection, uint32_t subscriptionId) noexcept;

    /// Get the server/client instance.
    ServerOrClient& getConnection() noexcept;
    /// Get the server/client instance.
    const ServerOrClient& getConnection() const noexcept;

    /// Get the server-assigned identifier of this subscription.
    uint32_t getSubscriptionId() const noexcept;

    /// Get all local monitored items.
    std::vector<MonitoredItem<ServerOrClient>> getMonitoredItems();

    /// Modify this subscription.
    /// @note Not implemented for Server.
    /// @see services::modifySubscription
    void setSubscriptionParameters(SubscriptionParameters& parameters);

    /// Enable/disable publishing of notification messages.
    /// @note Not implemented for Server.
    /// @see services::setPublishingMode
    void setPublishingMode(bool publishing);

    /// Create a monitored item for data change notifications (default settings).
    /// The monitoring mode is set to MonitoringMode::Reporting and the default open62541
    /// MonitoringParameters are used.
    /// @see services::MonitoringParameters
    MonitoredItem<ServerOrClient> subscribeDataChange(
        const NodeId& id, AttributeId attribute, DataChangeCallback<ServerOrClient> onDataChange
    );

    /// Create a monitored item for data change notifications.
    /// @copydetails services::MonitoringParameters
    MonitoredItem<ServerOrClient> subscribeDataChange(
        const NodeId& id,
        AttributeId attribute,
        MonitoringMode monitoringMode,
        MonitoringParameters& parameters,
        DataChangeCallback<ServerOrClient> onDataChange
    );

    /// Create a monitored item for event notifications (default settings).
    /// The monitoring mode is set to MonitoringMode::Reporting and the default open62541
    /// MonitoringParameters are used.
    /// @note Not implemented for Server.
    MonitoredItem<ServerOrClient> subscribeEvent(
        const NodeId& id, const EventFilter& eventFilter, EventCallback<ServerOrClient> onEvent
    );

    /// Create a monitored item for event notifications.
    /// @copydetails services::MonitoringParameters
    /// @note Not implemented for Server.
    MonitoredItem<ServerOrClient> subscribeEvent(
        const NodeId& id,
        MonitoringMode monitoringMode,
        MonitoringParameters& parameters,
        EventCallback<ServerOrClient> onEvent
    );

    /// Delete this subscription.
    /// @note Not implemented for Server.
    /// @see services::deleteSubscription
    void deleteSubscription();

private:
    ServerOrClient& connection_;
    uint32_t subscriptionId_{0U};
};

/* ---------------------------------------------------------------------------------------------- */

template <typename T>
inline bool operator==(const Subscription<T>& lhs, const Subscription<T>& rhs) noexcept {
    return (lhs.getConnection() == rhs.getConnection()) &&
           (lhs.getSubscriptionId() == rhs.getSubscriptionId());
}

template <typename T>
inline bool operator!=(const Subscription<T>& lhs, const Subscription<T>& rhs) noexcept {
    return !(lhs == rhs);
}

}  // namespace opcua

#endif
