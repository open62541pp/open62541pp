#pragma once

#include <cstdint>
#include <functional>

#include "open62541pp/config.hpp"
#include "open62541pp/types_composed.hpp"  // StatusChangeNotification

#ifdef UA_ENABLE_SUBSCRIPTIONS

namespace opcua {
class Client;
}  // namespace opcua

namespace opcua::services {

/**
 * @defgroup Subscription Subscription service set
 * Report notifications to the client.
 *
 * Note the difference between Subscriptions and MonitoredItems. Subscriptions are used to report
 * back notifications. MonitoredItems are used to generate notifications. Every MonitoredItem is
 * attached to exactly one Subscription. And a Subscription can contain many MonitoredItems.
 *
 * @see MonitoredItem
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13
 * @ingroup Services
 * @{
 */

/**
 * Subscription parameters with default values from open62541.
 */
struct SubscriptionParameters {
    /// Cyclic interval in milliseconds that the subscription is requested to return notifications.
    double publishingInterval = 500.0;
    /// Delete the subscription after defined publishing cycles without sending any notifications.
    uint32_t lifetimeCount = 10000;
    /// Send keep-alive message after defined publishing cycles without notifications.
    uint32_t maxKeepAliveCount = 10;
    /// The maximum number of notifications per publish (0 if unlimited).
    uint32_t maxNotificationsPerPublish = 0;
    /// Relative priority of the subscription. Notifications with higher priority are sent first.
    uint8_t priority = 0;
};

/**
 * @defgroup CreateSubscription CreateSubscription service
 * Create subscriptions. Subscriptions monitor a set of monitored items for notifications and return
 * them to the client.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.2
 * @{
 */

/**
 * Subscription deletion callback.
 * @param subId Subscription identifier
 */
using DeleteSubscriptionCallback = std::function<void(uint32_t subId)>;

/**s
 * Subscription status change notification callback.
 * @param subId Subscription identifier
 * @param notification Status change notification
 */
using StatusChangeNotificationCallback =
    std::function<void(uint32_t subId, StatusChangeNotification& notification)>;

/**
 * Create a subscription.
 * @param connection Instance of type Client
 * @param request Create subscription request
 * @param statusChangeCallback Invoked when the status of a subscription is changed
 * @param deleteCallback Invoked when the subscription is deleted
 */
[[nodiscard]] CreateSubscriptionResponse createSubscription(
    Client& connection,
    const CreateSubscriptionRequest& request,
    StatusChangeNotificationCallback statusChangeCallback = {},
    DeleteSubscriptionCallback deleteCallback = {}
);

/**
 * @overload
 */
[[nodiscard]] CreateSubscriptionResponse createSubscription(
    Client& connection,
    const SubscriptionParameters& parameters,
    bool publishingEnabled = true,
    StatusChangeNotificationCallback statusChangeCallback = {},
    DeleteSubscriptionCallback deleteCallback = {}
) noexcept;

/**
 * @}
 * @defgroup ModifySubscription ModifySubscription service
 * Modify subscriptions.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.3
 * @{
 */

/**
 * Modify a subscription.
 * @param connection Instance of type Client
 * @param request Modify subscription request
 */
ModifySubscriptionResponse modifySubscription(
    Client& connection, const ModifySubscriptionRequest& request
) noexcept;

/**
 * @overload
 */
ModifySubscriptionResponse modifySubscription(
    Client& connection, uint32_t subscriptionId, const SubscriptionParameters& parameters
) noexcept;

/**
 * @}
 * @defgroup SetPublishingMode SetPublishingMode service
 * Enable/disable sending of notifications on subscriptions.
 * Disable publishing of NotificationMessages of the subscription doesn't discontinue the sending
 * of keep-alive messages, nor change the monitoring mode.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.4
 * @{
 */

/**
 * Enable/disable publishing of notification messages.
 * @param connection Instance of type Client
 * @param request Set publishing mode request
 */
SetPublishingModeResponse setPublishingMode(
    Client& connection, const SetPublishingModeRequest& request
) noexcept;

/**
 * Enable/disable publishing of notification messages.
 * @param connection Instance of type Client
 * @param subscriptionId Identifier of the subscription returned by @ref createSubscription
 * @param publishing Enable/disable publishing
 */
StatusCode setPublishingMode(Client& connection, uint32_t subscriptionId, bool publishing) noexcept;

/**
 * @}
 * @defgroup DeleteSubscriptions DeleteSubscriptions service
 * Delete subscriptions.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.8
 * @{
 */

/**
 * Delete subscriptions.
 * @param connection Instance of type Client
 * @param request Delete subscriptions request
 */
DeleteSubscriptionsResponse deleteSubscriptions(
    Client& connection, const DeleteSubscriptionsRequest& request
) noexcept;

/**
 * Delete a single subscription.
 * @param connection Instance of type Client
 * @param subscriptionId Identifier of the subscription returned by @ref createSubscription
 */
StatusCode deleteSubscription(Client& connection, uint32_t subscriptionId) noexcept;

/**
 * @}
 * @}
 */

}  // namespace opcua::services

#endif
