#pragma once

#include <cstdint>
#include <functional>

#include "open62541pp/Config.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS

// forward declarations
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
 */

/**
 * Subscription parameters with default values from open62541.
 *
 * Parameters are passed by reference because illegal parameters can be revised by the server.
 * The updated parameters reflect the actual values that the server will use.
 * @ingroup Subscription
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
 * Subscription deletion callback.
 * @param subId Subscription identifier
 * @ingroup Subscription
 */
using DeleteSubscriptionCallback = std::function<void(uint32_t subId)>;

/**
 * Create a subscription.
 * @copydetails SubscriptionParameters
 *
 * @param client Instance of type Client
 * @param parameters Subscription parameters, may be revised by server
 * @param publishingEnabled Enable/disable publishing of the subscription
 * @param deleteCallback Invoked when the subscription is deleted
 * @returns Server-assigned identifier of the subscription
 * @ingroup Subscription
 */
[[nodiscard]] uint32_t createSubscription(
    Client& client,
    SubscriptionParameters& parameters,
    bool publishingEnabled = true,
    DeleteSubscriptionCallback deleteCallback = {}
);

/**
 * Modify a subscription.
 * @copydetails SubscriptionParameters
 *
 * @param client Instance of type Client
 * @param subscriptionId Identifier of the subscription returned by @ref createSubscription
 * @param parameters Subscription parameters, may be revised by server
 * @ingroup Subscription
 */
void modifySubscription(
    Client& client, uint32_t subscriptionId, SubscriptionParameters& parameters
);

/**
 * Enable/disable publishing of notification messages.
 * Disable publishing of NotificationMessages of the subscription doesn't discontinue the sending
 * of keep-alive messages, nor change the monitoring mode.
 *
 * @param client Instance of type Client
 * @param subscriptionId Identifier of the subscription returned by @ref createSubscription
 * @param publishing Enable/disable publishing
 * @ingroup Subscription
 */
void setPublishingMode(Client& client, uint32_t subscriptionId, bool publishing);

/**
 * Delete a subscription.
 *
 * @param client Instance of type Client
 * @param subscriptionId Identifier of the subscription returned by @ref createSubscription
 * @ingroup Subscription
 */
void deleteSubscription(Client& client, uint32_t subscriptionId);

}  // namespace opcua::services

#endif
