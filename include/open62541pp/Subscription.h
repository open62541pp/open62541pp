#pragma once

#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>  // move
#include <vector>

#include "open62541pp/Common.h"  // AttributeId, MonitoringMode
#include "open62541pp/Config.h"
#include "open62541pp/MonitoredItem.h"
#include "open62541pp/Span.h"
#include "open62541pp/services/MonitoredItem.h"
#include "open62541pp/services/Subscription.h"
#include "open62541pp/types/ExtensionObject.h"
#include "open62541pp/types/NodeId.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS

namespace opcua {

// forward declarations
class DataValue;
class EventFilter;
class Server;
class Variant;

using SubscriptionParameters = services::SubscriptionParameters;
using MonitoringParametersEx = services::MonitoringParametersEx;
using DataChangeNotificationCallback = services::DataChangeNotificationCallback;
using EventNotificationCallback = services::EventNotificationCallback;

/// Data change notification callback.
/// @deprecated Use DataChangeNotificationCallback instead
/// @tparam T Server or Client
template <typename T>
using DataChangeCallback =
    std::function<void(const MonitoredItem<T>& item, const DataValue& value)>;

/// Event notification callback.
/// @deprecated Use EventNotificationCallback instead
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
 * Use the free functions in the opcua::services namespace for more advanced usage:
 * - @ref Subscription
 * - @ref MonitoredItem
 */
template <typename Connection>
class Subscription {
public:
    /// Wrap an existing subscription.
    /// The `subscriptionId` is ignored and set to `0U` for servers.
    Subscription(Connection& connection, uint32_t subscriptionId) noexcept
        : connection_(connection),
          subscriptionId_(std::is_same_v<Connection, Server> ? 0U : subscriptionId) {}

    /// Get the server/client instance.
    Connection& connection() noexcept {
        return connection_;
    }

    /// Get the server/client instance.
    const Connection& connection() const noexcept {
        return connection_;
    }

    [[deprecated("Use connection() instead")]]
    Connection& getConnection() noexcept {
        return connection_;
    }

    [[deprecated("Use connection() instead")]]
    const Connection& getConnection() const noexcept {
        return connection_;
    }

    /// Get the server-assigned identifier of this subscription.
    uint32_t subscriptionId() const noexcept {
        return subscriptionId_;
    }

    [[deprecated("Use subscriptionId() instead")]]
    uint32_t getSubscriptionId() const noexcept {
        return subscriptionId_;
    }

    /// Get all local monitored items.
    std::vector<MonitoredItem<Connection>> getMonitoredItems();

    /// Modify this subscription.
    /// @note Not implemented for Server.
    /// @see services::modifySubscription
    void setSubscriptionParameters(SubscriptionParameters& parameters) {
        services::modifySubscription(connection_, subscriptionId_, parameters);
    }

    /// Enable/disable publishing of notification messages.
    /// @note Not implemented for Server.
    /// @see services::setPublishingMode
    void setPublishingMode(bool publishing) {
        services::setPublishingMode(connection_, subscriptionId_, publishing);
    }

    /// Create a monitored item for data change notifications.
    /// @copydetails services::MonitoringParametersEx
    MonitoredItem<Connection> subscribeDataChange(
        const NodeId& id,
        AttributeId attribute,
        MonitoringMode monitoringMode,
        MonitoringParametersEx& parameters,
        DataChangeNotificationCallback onDataChange
    ) {
        const uint32_t monitoredItemId = services::createMonitoredItemDataChange(
            connection_,
            subscriptionId_,
            {id, attribute},
            monitoringMode,
            parameters,
            std::move(onDataChange)
        );
        return {connection_, subscriptionId_, monitoredItemId};
    }

    /// @deprecated Use overload with DataChangeNotificationCallback instead
    [[deprecated("Use overload with DataChangeNotificationCallback instead")]]
    MonitoredItem<Connection> subscribeDataChange(
        const NodeId& id,
        AttributeId attribute,
        MonitoringMode monitoringMode,
        MonitoringParametersEx& parameters,
        DataChangeCallback<Connection> onDataChange
    ) {
        return subscribeDataChange(
            id, attribute, monitoringMode, parameters, createCallback(std::move(onDataChange))
        );
    }

    /// Create a monitored item for data change notifications (default settings).
    /// The monitoring mode is set to MonitoringMode::Reporting and the default open62541
    /// MonitoringParametersEx are used.
    /// @see services::MonitoringParametersEx
    MonitoredItem<Connection> subscribeDataChange(
        const NodeId& id, AttributeId attribute, DataChangeNotificationCallback onDataChange
    ) {
        MonitoringParametersEx parameters;
        return subscribeDataChange(
            id, attribute, MonitoringMode::Reporting, parameters, std::move(onDataChange)
        );
    }

    /// @deprecated Use overload with DataChangeNotificationCallback instead
    [[deprecated("Use overload with DataChangeNotificationCallback instead")]]
    MonitoredItem<Connection> subscribeDataChange(
        const NodeId& id, AttributeId attribute, DataChangeCallback<Connection> onDataChange
    ) {
        return subscribeDataChange(id, attribute, createCallback(std::move(onDataChange)));
    }

    /// Create a monitored item for event notifications.
    /// @copydetails services::MonitoringParametersEx
    /// @note Not implemented for Server.
    MonitoredItem<Connection> subscribeEvent(
        const NodeId& id,
        MonitoringMode monitoringMode,
        MonitoringParametersEx& parameters,
        EventNotificationCallback onEvent  // NOLINT(*-unnecessary-value-param), false positive?
    ) {
        const uint32_t monitoredItemId = services::createMonitoredItemEvent(
            connection_,
            subscriptionId_,
            {id, AttributeId::EventNotifier},
            monitoringMode,
            parameters,
            std::move(onEvent)
        );
        return {connection_, subscriptionId_, monitoredItemId};
    }

    /// @deprecated Use overload with EventNotificationCallback instead
    [[deprecated("Use overload with EventNotificationCallback instead")]]
    MonitoredItem<Connection> subscribeEvent(
        const NodeId& id,
        MonitoringMode monitoringMode,
        MonitoringParametersEx& parameters,
        EventCallback<Connection> onEvent
    ) {
        return subscribeEvent(id, monitoringMode, parameters, createCallback(std::move(onEvent)));
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

    /// @deprecated Use overload with EventNotificationCallback instead
    [[deprecated("Use overload with EventNotificationCallback instead")]]
    MonitoredItem<Connection> subscribeEvent(
        const NodeId& id, const EventFilter& eventFilter, EventCallback<Connection> onEvent
    ) {
        return subscribeEvent(id, eventFilter, createCallback(std::move(onEvent)));
    }

    /// Delete this subscription.
    /// @note Not implemented for Server.
    void deleteSubscription() {
        services::deleteSubscription(connection_, subscriptionId_);
    }

private:
    DataChangeNotificationCallback createCallback(DataChangeCallback<Connection> onDataChange) {
        return [connectionPtr = &connection_, callback = std::move(onDataChange)](
                   uint32_t subId, uint32_t monId, const DataValue& value
               ) {
            const MonitoredItem<Connection> monitoredItem(*connectionPtr, subId, monId);
            callback(monitoredItem, value);
        };
    }

    EventNotificationCallback createCallback(EventCallback<Connection> onEvent) {
        return [connectionPtr = &connection_, callback = std::move(onEvent)](
                   uint32_t subId, uint32_t monId, Span<const Variant> eventFields
               ) {
            const MonitoredItem<Connection> monitoredItem(*connectionPtr, subId, monId);
            callback(monitoredItem, eventFields);
        };
    }

    Connection& connection_;
    uint32_t subscriptionId_{0U};
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
