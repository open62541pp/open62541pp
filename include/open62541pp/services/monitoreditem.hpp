#pragma once

#include <cstdint>
#include <functional>

#include "open62541pp/common.hpp"  // TimestampsToReturn, MonitoringMode
#include "open62541pp/config.hpp"
#include "open62541pp/result.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/types_composed.hpp"

#ifdef UA_ENABLE_SUBSCRIPTIONS

namespace opcua {
class Client;
}  // namespace opcua

namespace opcua::services {

/**
 * @defgroup MonitoredItem MonitoredItem service set
 * Subscribe to data and events.
 *
 * Note the difference between Subscriptions and MonitoredItems. Subscriptions are used to report
 * back notifications. Monitored items are used to generate notifications. Every monitored item is
 * attached to exactly one subscription. And a subscription can contain many monitored items.
 *
 * Monitored items can also be registered locally (server-side). Notifications are then forwarded
 * to a user-defined callback instead of a remote client.
 *
 * @see Subscription
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12
 * @ingroup Services
 * @{
 */

/**
 * Extended monitoring parameters with default values from open62541.
 * This is an extended version of `UA_MonitoringParameters` with the `timestamps` parameter.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.21
 */
struct MonitoringParametersEx {
    /// Timestamps to be transmitted. Won't be revised by the server.
    TimestampsToReturn timestamps = TimestampsToReturn::Both;
    /// Interval in milliseconds that defines the fastest rate at which the MonitoredItem should be
    /// accessed and evaluated. The following values have special meaning:
    /// - `0.0` to use the fastest practical rate
    /// - `-1.0` to use the default sampling interval (publishing interval of the subscription)
    double samplingInterval = 250.0;
    /// Filter is used by the server to determine if the MonitoredItem should generate
    /// notifications. The filter parameter type is an extensible parameter type and can be, for
    /// example, of type DataChangeFilter, EventFilter or AggregateFilter.
    /// @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.22
    ExtensionObject filter;
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
 * @defgroup CreateMonitoredItems CreateMonitoredItems service
 * Create and add a monitored item to a subscription.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.2
 * @{
 */

/**
 * MonitoredItem deletion callback.
 * @param subId Subscription identifier
 * @param monId MonitoredItem identifier
 */
using DeleteMonitoredItemCallback = std::function<void(uint32_t subId, uint32_t monId)>;

/**
 * Data change notification callback.
 * @param subId Subscription identifier (`0U` for local (server-side) monitored item)
 * @param monId MonitoredItem identifier
 * @param value Changed value
 */
using DataChangeNotificationCallback =
    std::function<void(uint32_t subId, uint32_t monId, const DataValue& value)>;

/**
 * Event notification callback.
 * @param subId Subscription identifier (`0U` for local (server-side) monitored item)
 * @param monId MonitoredItem identifier
 * @param eventFields Event fields
 */
using EventNotificationCallback =
    std::function<void(uint32_t subId, uint32_t monId, Span<const Variant> eventFields)>;

/**
 * Create and add monitored items to a subscription for data change notifications.
 * Don't use this function to monitor the `EventNotifier` attribute.
 * Create monitored items with @ref createMonitoredItemsEvent instead.
 *
 * @param connection Instance of type Client
 * @param request Create monitored items request
 * @param dataChangeCallback Invoked when the monitored item is changed
 * @param deleteCallback Invoked when the monitored item is deleted
 */
CreateMonitoredItemsResponse createMonitoredItemsDataChange(
    Client& connection,
    const CreateMonitoredItemsRequest& request,
    DataChangeNotificationCallback dataChangeCallback,
    DeleteMonitoredItemCallback deleteCallback = {}
);

/**
 * Create and add a monitored item to a subscription for data change notifications.
 * Don't use this function to monitor the `EventNotifier` attribute.
 * Create a monitored item with @ref createMonitoredItemEvent instead.
 *
 * @return Server-assigned identifier of the monitored item
 * @param connection Instance of type Server or Client
 * @param subscriptionId Identifier of the subscription returned by @ref createSubscription.
 *                       Use `0U` for a local server-side monitored item.
 * @param itemToMonitor Item to monitor
 * @param monitoringMode Monitoring mode
 * @param parameters Monitoring parameters, may be revised by server
 * @param dataChangeCallback Invoked when the monitored item is changed
 * @param deleteCallback Invoked when the monitored item is deleted
 */
template <typename T>
[[nodiscard]] Result<uint32_t> createMonitoredItemDataChange(
    T& connection,
    uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    const MonitoringParametersEx& parameters,
    DataChangeNotificationCallback dataChangeCallback,
    DeleteMonitoredItemCallback deleteCallback = {}
);

/**
 * Create and add monitored items to a subscription for event notifications.
 * The `attributeId` of ReadValueId must be set to AttributeId::EventNotifier.
 *
 * @param connection Instance of type Client
 * @param request Create monitored items request
 * @param eventCallback Invoked when an event is published
 * @param deleteCallback Invoked when the monitored item is deleted
 */
CreateMonitoredItemsResponse createMonitoredItemsEvent(
    Client& connection,
    const CreateMonitoredItemsRequest& request,
    EventNotificationCallback eventCallback,
    DeleteMonitoredItemCallback deleteCallback = {}
);

/**
 * Create and add a monitored item to a subscription for event notifications.
 * The `attributeId` of ReadValueId must be set to AttributeId::EventNotifier.
 *
 * @return Server-assigned identifier of the monitored item
 * @param connection Instance of type Client
 * @param subscriptionId Identifier of the subscription returned by @ref createSubscription
 * @param itemToMonitor Item to monitor
 * @param monitoringMode Monitoring mode
 * @param parameters Monitoring parameters, may be revised by server
 * @param eventCallback Invoked when an event is published
 * @param deleteCallback Invoked when the monitored item is deleted
 */
[[nodiscard]] Result<uint32_t> createMonitoredItemEvent(
    Client& connection,
    uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    const MonitoringParametersEx& parameters,
    EventNotificationCallback eventCallback,
    DeleteMonitoredItemCallback deleteCallback = {}
);

/**
 * @}
 * @defgroup ModifyMonitoredItems ModifyMonitoredItems service
 * Modify a monitored items of a subscription.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.3
 * @{
 */

/**
 * Modify monitored items of a subscription.
 *
 * @param connection Instance of type Client
 * @param request Modify monitored items request
 */
ModifyMonitoredItemsResponse modifyMonitoredItems(
    Client& connection, const ModifyMonitoredItemsRequest& request
) noexcept;

/**
 * Modify a monitored item of a subscription.
 *
 * @param connection Instance of type Client
 * @param subscriptionId Identifier of the subscription returned by @ref createSubscription
 * @param monitoredItemId Identifier of the monitored item
 * @param parameters Monitoring parameters, may be revised by server
 */
Result<void> modifyMonitoredItem(
    Client& connection,
    uint32_t subscriptionId,
    uint32_t monitoredItemId,
    const MonitoringParametersEx& parameters
) noexcept;

/**
 * @}
 * @defgroup SetMonitoringMode SetMonitoringMode service
 * Set the monitoring mode of a monitored items.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.4
 * @{
 */

/**
 * Set the monitoring mode of monitored items.
 *
 * @param connection Instance of type Client
 * @param request Set monitoring mode request
 */
SetMonitoringModeResponse setMonitoringMode(
    Client& connection, const SetMonitoringModeRequest& request
) noexcept;

/**
 * Set the monitoring mode of a monitored item.
 *
 * @param connection Instance of type Client
 * @param subscriptionId Identifier of the subscription returned by @ref createSubscription
 * @param monitoredItemId Identifier of the monitored item
 * @param monitoringMode Monitoring mode
 */
Result<void> setMonitoringMode(
    Client& connection,
    uint32_t subscriptionId,
    uint32_t monitoredItemId,
    MonitoringMode monitoringMode
) noexcept;

/**
 * @}
 * @defgroup SetTriggering SetTriggering service
 * Create and delete triggering links for a triggering item.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.1.6
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.5
 * @{
 */

/**
 * Add and delete triggering links of monitored items.
 * The triggering item and the items to report shall belong to the same subscription.
 * @note Supported since open62541 v1.2
 *
 * @param connection Instance of type Client
 * @param request Set triggering request
 */
SetTriggeringResponse setTriggering(
    Client& connection, const SetTriggeringRequest& request
) noexcept;

/**
 * Add and delete triggering links of a monitored item.
 * The triggering item and the items to report shall belong to the same subscription.
 * @note Supported since open62541 v1.2
 *
 * @param connection Instance of type Client
 * @param subscriptionId Identifier of the subscription returned by @ref createSubscription
 * @param triggeringItemId Identifier of the triggering monitored item
 * @param linksToAdd List of monitoring item identifiers to be added as triggering links
 * @param linksToRemove List of monitoring item identifiers to be removed as triggering links
 */
Result<void> setTriggering(
    Client& connection,
    uint32_t subscriptionId,
    uint32_t triggeringItemId,
    Span<const uint32_t> linksToAdd,
    Span<const uint32_t> linksToRemove
) noexcept;

/**
 * @}
 * @defgroup DeleteMonitoredItems DeleteMonitoredItems service
 * Delete a monitored items from subscriptions.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.6
 * @{
 */

/**
 * Delete monitored items from subscriptions.
 *
 * @param connection Instance of type Client
 * @param request Delete monitored items request
 */
DeleteMonitoredItemsResponse deleteMonitoredItems(
    Client& connection, const DeleteMonitoredItemsRequest& request
) noexcept;

/**
 * Delete a monitored item from a subscription.
 *
 * @param connection Instance of type Server or Client
 * @param subscriptionId Identifier of the subscription returned by @ref createSubscription.
 *                       Use `0U` for a local server-side monitored item.
 * @param monitoredItemId Identifier of the monitored item
 */
template <typename T>
Result<void> deleteMonitoredItem(T& connection, uint32_t subscriptionId, uint32_t monitoredItemId);

/**
 * @}
 * @}
 */

}  // namespace opcua::services

#endif
