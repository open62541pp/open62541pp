#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "open62541pp/Common.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/Variant.h"

// forward declarations
namespace opcua {
class Client;
class Server;
}  // namespace opcua

namespace opcua::services {

/**
 * @defgroup MonitoredItem MonitoredItem service set
 * Subscribe to data and events.
 *
 * Note the difference between Subscriptions and MonitoredItems. Subscriptions are used to report
 * back notifications. MonitoredItems are used to generate notifications. Every MonitoredItem is
 * attached to exactly one Subscription. And a Subscription can contain many MonitoredItems.
 *
 * @see Subscription
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12
 * @ingroup Services
 */

/**
 * Monitoring parameters with default values from open62541.
 * Parameters are passed by reference because illegal parameters can be revised by the server.
 * The updated parameters reflect the actual values that the server will use.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.21
 * @ingroup MonitoredItem
 */
struct MonitoringParameters {
    /// Timestamps to be transmitted. Won't be revised by the server.
    TimestampsToReturn timestamps = TimestampsToReturn::Both;
    /// Interval in milliseconds that defines the fastest rate at which the MonitoredItem should be
    /// accessed and evaluated.
    double samplingInterval = 250.0;
    /// Size of the MonitoringItem queue.
    /// The following values have special meaning:
    /// - `0` to retrieve the server's default queue size
    /// - `1` to retrieve the server's minimum queue size
    /// In the case of a queue overflow, an Event of the type EventQueueOverflowEventType is
    /// generated.
    uint32_t queueSize = 1;
    /// Discard policy when the queue is full.
    /// - `true`: the oldest (first) notification in the queue is discarded
    /// - `false`: the last notification added to the queue gets replaced with the new notification
    bool discardOldest = true;
};

/**
 * MonitoredItem deletion callback.
 * @param subId Subscription identifier
 * @param monId MonitoredItem identifier
 * @ingroup MonitoredItem
 */
using DeleteMonitoredItemCallback = std::function<void(uint32_t subId, uint32_t monId)>;

/**
 * Data change notification callback.
 * @param subId Subscription identifier
 * @param monId MonitoredItem identifier
 * @param value Changed value
 * @ingroup MonitoredItem
 */
using DataChangeNotificationCallback =
    std::function<void(uint32_t subId, uint32_t monId, const DataValue& value)>;

/**
 * Event notification callback.
 * @param subId Subscription identifier
 * @param monId MonitoredItem identifier
 * @param value Changed value
 * @ingroup MonitoredItem
 */
using EventNotificationCallback =
    std::function<void(uint32_t subId, uint32_t monId, const std::vector<Variant>& eventFields)>;

/**
 * Create and add a monitored item to a subscription for data change notifications.
 * Don't use this function to monitor the `EventNotifier` attribute.
 * Create a monitored item with @ref createMonitoredItemEvent instead.
 * @copydetails MonitoringParameters
 *
 * @param client Instance of type Client
 * @param subscriptionId Identifier for the subscription returned by @ref createSubscription
 * @param itemToMonitor Item to monitor
 * @param monitoringMode Monitoring mode
 * @param parameters Monitoring parameters, may be revised by server
 * @param dataChangeCallback Invoked when the monitored item is changed
 * @param deleteCallback Invoked when the monitored item is deleted
 * @returns Server-assigned identifier for the monitored item
 * @ingroup MonitoredItem
 */
[[nodiscard]] uint32_t createMonitoredItemDataChange(
    Client& client,
    uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    MonitoringParameters& parameters,
    DataChangeNotificationCallback dataChangeCallback,
    DeleteMonitoredItemCallback deleteCallback = {}
);

/**
 * Create a local monitored item for data change notifications.
 * Don't use this function to monitor the `EventNotifier` attribute.
 * @copydetails MonitoringParameters
 *
 * @param server Instance of type Server
 * @param itemToMonitor Item to monitor
 * @param monitoringMode Monitoring mode
 * @param parameters Monitoring parameters, may be revised by server
 * @param dataChangeCallback Invoked when the monitored item is changed
 * @returns Server-assigned identifier for the monitored item
 * @ingroup MonitoredItem
 */
[[nodiscard]] uint32_t createMonitoredItemDataChange(
    Server& server,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    MonitoringParameters& parameters,
    DataChangeNotificationCallback dataChangeCallback
);

/**
 * Create and add a monitored item to a subscription for event notifications.
 * The `attributeId` of ReadValueId must be set to AttributeId::EventNotifier.
 * @copydetails MonitoringParameters
 *
 * @param client Instance of type Client
 * @param subscriptionId Identifier for the subscription returned by @ref createSubscription
 * @param itemToMonitor Item to monitor
 * @param monitoringMode Monitoring mode
 * @param parameters Monitoring parameters, may be revised by server
 * @param eventCallback Invoked when an event is published
 * @param deleteCallback Invoked when the monitored item is deleted
 * @returns Server-assigned identifier for the monitored item
 * @ingroup MonitoredItem
 */
[[nodiscard]] uint32_t createMonitoredItemEvent(
    Client& client,
    uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    MonitoringParameters& parameters,
    EventNotificationCallback eventCallback,
    DeleteMonitoredItemCallback deleteCallback = {}
);

/**
 * Modify a monitored item of a subscription.
 * @copydetails MonitoringParameters
 *
 * @param client Instance of type Client
 * @param subscriptionId Identifier for the subscription returned by @ref createSubscription
 * @param monitoredItemId Identifier for the monitored item
 * @param parameters Monitoring parameters, may be revised by server
 * @ingroup MonitoredItem
 */
void modifyMonitoredItem(
    Client& client,
    uint32_t subscriptionId,
    uint32_t monitoredItemId,
    MonitoringParameters& parameters
);

/**
 * Set the monitoring mode of a monitored item.
 *
 * @param client Instance of type Client
 * @param subscriptionId Identifier for the subscription returned by @ref createSubscription
 * @param monitoredItemId Identifier for the monitored item
 * @param monitoringMode Monitoring mode
 * @ingroup MonitoredItem
 */
void setMonitoringMode(
    Client& client, uint32_t subscriptionId, uint32_t monitoredItemId, MonitoringMode monitoringMode
);

/**
 * Add and delete triggering links for a monitored item.
 * The triggering item and the items to report shall belong to the same subscription.
 *
 * @note Supported since open62541 v1.2
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.1.6
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.5
 *
 * @param client Instance of type Client
 * @param subscriptionId Identifier for the subscription returned by @ref createSubscription
 * @param triggeringItemId Identifier for the triggering monitored item
 * @param linksToAdd List of monitoring item identifiers to be added as triggering links
 * @param linksToRemove List of monitoring item identifiers to be removed as triggering links
 * @ingroup MonitoredItem
 */
void setTriggering(
    Client& client,
    uint32_t subscriptionId,
    uint32_t triggeringItemId,
    const std::vector<uint32_t>& linksToAdd,
    const std::vector<uint32_t>& linksToRemove
);

/**
 * Delete a monitored item from a subscription.
 *
 * @param client Instance of type Server or Client
 * @param subscriptionId Identifier for the subscription returned by @ref createSubscription
 * @param monitoredItemId Identifier for the monitored item
 * @ingroup MonitoredItem
 */
void deleteMonitoredItem(Client& client, uint32_t subscriptionId, uint32_t monitoredItemId);

/**
 * Delete a local monitored item.
 *
 * @param server Instance of type Server
 * @param monitoredItemId Identifier for the monitored item
 * @ingroup MonitoredItem
 */
void deleteMonitoredItem(Server& server, uint32_t monitoredItemId);

}  // namespace opcua::services
