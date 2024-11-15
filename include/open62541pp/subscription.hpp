#pragma once

#include <type_traits>
#include <utility>  // move
#include <vector>

#include "open62541pp/common.hpp"  // AttributeId, IntegerId, MonitoringMode
#include "open62541pp/config.hpp"
#include "open62541pp/monitoreditem.hpp"
#include "open62541pp/services/monitoreditem.hpp"
#include "open62541pp/services/subscription.hpp"
#include "open62541pp/types.hpp"

#ifdef UA_ENABLE_SUBSCRIPTIONS

namespace opcua {

class EventFilter;
class Server;

using SubscriptionParameters = services::SubscriptionParameters;
using MonitoringParametersEx = services::MonitoringParametersEx;
using DataChangeNotificationCallback = services::DataChangeNotificationCallback;
using EventNotificationCallback = services::EventNotificationCallback;

/**
 * High-level subscription class.
 *
 * The API is symmetric for both Server and Client, although servers don't use the subscription
 * mechanism of OPC UA to transport notifications of data changes and events. Instead MonitoredItems
 * are registered locally. Notifications are then forwarded to user-defined callbacks instead of a
 * remote client. The `subscriptionId` for servers is always `0U`.
 *
 * @tparam Connection Server or Client
 * @note Not all methods are available and implemented for servers.
 *
 * Use the free functions in the opcua::services namespace for more advanced usage:
 * - @ref Subscription
 * - @ref MonitoredItem
 */
template <typename Connection>
class Subscription {
public:
    /// Wrap an existing subscription.
    /// The `subscriptionId` is ignored and set to `0U` for servers.
    Subscription(Connection& connection, IntegerId subscriptionId) noexcept
        : connection_(&connection),
          subscriptionId_(std::is_same_v<Connection, Server> ? 0U : subscriptionId) {}

    /// Get the server/client instance.
    Connection& connection() noexcept {
        return *connection_;
    }

    /// Get the server/client instance.
    const Connection& connection() const noexcept {
        return *connection_;
    }

    /// Get the server-assigned identifier of this subscription.
    IntegerId subscriptionId() const noexcept {
        return subscriptionId_;
    }

    /// Get all local monitored items.
    std::vector<MonitoredItem<Connection>> getMonitoredItems();

    /// Modify this subscription.
    /// @note Not implemented for Server.
    /// @see services::modifySubscription
    void setSubscriptionParameters(const SubscriptionParameters& parameters) {
        const auto response = services::modifySubscription(
            connection(), subscriptionId(), parameters
        );
        response.getResponseHeader().getServiceResult().throwIfBad();
    }

    /// Enable/disable publishing of notification messages.
    /// @note Not implemented for Server.
    /// @see services::setPublishingMode
    void setPublishingMode(bool publishing) {
        services::setPublishingMode(connection(), subscriptionId(), publishing).throwIfBad();
    }

    /// Create a monitored item for data change notifications.
    MonitoredItem<Connection> subscribeDataChange(
        const NodeId& id,
        AttributeId attribute,
        MonitoringMode monitoringMode,
        const MonitoringParametersEx& parameters,
        DataChangeNotificationCallback onDataChange
    ) {
        const auto result = services::createMonitoredItemDataChange(
            connection(),
            subscriptionId(),
            {id, attribute},
            monitoringMode,
            parameters,
            std::move(onDataChange),
            {}
        );
        result.getStatusCode().throwIfBad();
        return {connection(), subscriptionId(), result.getMonitoredItemId()};
    }

    /// Create a monitored item for data change notifications (default settings).
    /// The monitoring mode is set to MonitoringMode::Reporting and the default open62541
    /// MonitoringParametersEx are used.
    MonitoredItem<Connection> subscribeDataChange(
        const NodeId& id, AttributeId attribute, DataChangeNotificationCallback onDataChange
    ) {
        const MonitoringParametersEx parameters;
        return subscribeDataChange(
            id, attribute, MonitoringMode::Reporting, parameters, std::move(onDataChange)
        );
    }

    /// Create a monitored item for event notifications.
    /// @note Not implemented for Server.
    MonitoredItem<Connection> subscribeEvent(
        const NodeId& id,
        MonitoringMode monitoringMode,
        const MonitoringParametersEx& parameters,
        EventNotificationCallback onEvent  // NOLINT(*-unnecessary-value-param), false positive?
    ) {
        const auto result = services::createMonitoredItemEvent(
            connection(),
            subscriptionId(),
            {id, AttributeId::EventNotifier},
            monitoringMode,
            parameters,
            std::move(onEvent)
        );
        result.getStatusCode().throwIfBad();
        return {connection(), subscriptionId(), result.getMonitoredItemId()};
    }

    /// Create a monitored item for event notifications (default settings).
    /// The monitoring mode is set to MonitoringMode::Reporting and the default open62541
    /// MonitoringParametersEx are used.
    /// @note Not implemented for Server.
    MonitoredItem<Connection> subscribeEvent(
        const NodeId& id, const EventFilter& eventFilter, EventNotificationCallback onEvent
    ) {
        MonitoringParametersEx parameters;
        parameters.filter = ExtensionObject::fromDecodedCopy(eventFilter);
        return subscribeEvent(id, MonitoringMode::Reporting, parameters, std::move(onEvent));
    }

    /// Delete this subscription.
    /// @note Not implemented for Server.
    void deleteSubscription() {
        services::deleteSubscription(connection(), subscriptionId()).throwIfBad();
    }

private:
    Connection* connection_;
    IntegerId subscriptionId_{0U};
};

/* ---------------------------------------------------------------------------------------------- */

template <typename T>
inline bool operator==(const Subscription<T>& lhs, const Subscription<T>& rhs) noexcept {
    return (lhs.connection() == rhs.connection()) && (lhs.subscriptionId() == rhs.subscriptionId());
}

template <typename T>
inline bool operator!=(const Subscription<T>& lhs, const Subscription<T>& rhs) noexcept {
    return !(lhs == rhs);
}

}  // namespace opcua

#endif
