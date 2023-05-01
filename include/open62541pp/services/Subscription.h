#pragma once

#include <cstdint>
#include <functional>

#include "open62541pp/Common.h"

namespace opcua::services {

/**
 * @defgroup Subscription Subscription service set
 * Report notifications to the client.
 *
 * Note the difference between Subscriptions and MonitoredItems. Subscriptions are used to report
 * back notifications. MonitoredItems are used to generate notifications. Every MonitoredItem is
 * attached to exactly one Subscription. And a Subscription can contain many MonitoredItems.
 *
 * @note
 * The subscription mechanism does not exist within a server. Instead, MonitoredItems can be
 * registered locally. Because the API is symmetric for both clients and servers, the subscription
 * mechanism for servers is simulated. Please refer to the function documentation for details:
 * - @ref createSubscription
 * - @ref modifySubscription
 * - @ref setPublishingMode
 * - @ref deleteSubscription
 *
 * @see https://github.com/open62541/open62541/blob/v1.3.5/include/open62541/server.h#L1049-L1100
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
 *
 * @note Simulated subscription mechanism with Server: Always returns `0`.
 *
 * @copydetails SubscriptionParameters
 * @param serverOrClient Instance of type Server or Client
 * @param parameters Subscription parameters, may be revised by server
 * @param publishingEnabled Enable/disable publishing for the subscription
 * @param deleteCallback Invoked when the subscription is deleted
 * @returns Server-assigned identifier for the subscription
 * @ingroup Subscription
 */
template <typename T>
[[nodiscard]] uint32_t createSubscription(
    T& serverOrClient,
    SubscriptionParameters& parameters,
    bool publishingEnabled = true,
    DeleteSubscriptionCallback deleteCallback = {}
);

/**
 * Modify a subscription.
 *
 * @note Simulated subscription mechanism with Server: Does nothing.
 *
 * @copydetails SubscriptionParameters
 * @param serverOrClient Instance of type Server or Client
 * @param subscriptionId Identifier for the subscription returned by @ref createSubscription
 * @param parameters Subscription parameters, may be revised by server
 * @ingroup Subscription
 */
template <typename T>
void modifySubscription(
    T& serverOrClient, uint32_t subscriptionId, SubscriptionParameters& parameters
);

/**
 * Enable/disable publishing of NotificationMessages for the subscription.
 *
 * @note Simulated subscription mechanism with Server: Does nothing.
 *
 * Disable publishing of NotificationMessages for the subscription doesn't discontinue the sending
 * of keep-alive messages, nor change the monitoring mode.
 * @param serverOrClient Instance of type Server or Client
 * @param subscriptionId Identifier for the subscription returned by @ref createSubscription
 * @param publishing Enable/disable publishing
 * @ingroup Subscription
 */
template <typename T>
void setPublishingMode(T& serverOrClient, uint32_t subscriptionId, bool publishing);

/**
 * Delete a subscription.
 *
 * @note Simulated subscription mechanism with Server: Delete all local monitored items.
 *
 * @param serverOrClient Instance of type Server or Client
 * @param subscriptionId Identifier for the subscription returned by @ref createSubscription
 * @ingroup Subscription
 */
template <typename T>
void deleteSubscription(T& serverOrClient, uint32_t subscriptionId);

}  // namespace opcua::services
