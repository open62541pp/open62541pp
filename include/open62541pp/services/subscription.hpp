#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <utility>  // forward

#include "open62541pp/async.hpp"
#include "open62541pp/config.hpp"
#include "open62541pp/detail/client_utils.hpp"  // getHandle
#include "open62541pp/detail/open62541/client.h"
#include "open62541pp/exception.hpp"
#include "open62541pp/services/detail/async_hook.hpp"
#include "open62541pp/services/detail/async_transform.hpp"
#include "open62541pp/services/detail/client_service.hpp"
#include "open62541pp/services/detail/request_handling.hpp"
#include "open62541pp/services/detail/response_handling.hpp"
#include "open62541pp/services/detail/subscription_context.hpp"
#include "open62541pp/ua/types.hpp"  // IntegerId, StatusChangeNotification

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
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.14
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
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.14.2
 * @{
 */

/**
 * Subscription deletion callback.
 * @param subId Subscription identifier
 */
using DeleteSubscriptionCallback = std::function<void(IntegerId subId)>;

/**
 * Subscription status change notification callback.
 * @param subId Subscription identifier
 * @param notification Status change notification
 */
using StatusChangeNotificationCallback =
    std::function<void(IntegerId subId, StatusChangeNotification& notification)>;

namespace detail {
std::unique_ptr<SubscriptionContext> createSubscriptionContext(
    Client& connection,
    StatusChangeNotificationCallback&& statusChangeCallback,
    DeleteSubscriptionCallback&& deleteCallback
);

void storeSubscriptionContext(
    Client& connection, IntegerId subscriptionId, std::unique_ptr<SubscriptionContext>&& context
);
}  // namespace detail

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
    StatusChangeNotificationCallback statusChangeCallback,
    DeleteSubscriptionCallback deleteCallback
);

/// @overload
[[nodiscard]] CreateSubscriptionResponse createSubscription(
    Client& connection,
    const SubscriptionParameters& parameters,
    bool publishingEnabled,
    StatusChangeNotificationCallback statusChangeCallback,
    DeleteSubscriptionCallback deleteCallback
) noexcept;

#if UAPP_HAS_ASYNC_SUBSCRIPTIONS
/**
 * @copydoc createSubscription(Client&, const CreateSubscriptionRequest&,
 *          StatusChangeNotificationCallback, DeleteSubscriptionCallback)
 * @param token @completiontoken{void(CreateSubscriptionResponse&)}
 * @return @asyncresult{CreateSubscriptionResponse}
 */
template <typename CompletionToken>
auto createSubscriptionAsync(
    Client& connection,
    const CreateSubscriptionRequest& request,
    StatusChangeNotificationCallback statusChangeCallback,
    DeleteSubscriptionCallback deleteCallback,
    CompletionToken&& token
) {
    auto context = detail::createSubscriptionContext(
        connection, std::move(statusChangeCallback), std::move(deleteCallback)
    );
    return detail::AsyncServiceAdapter<CreateSubscriptionResponse>::initiate(
        connection,
        [&, context = context.get()](UA_ClientAsyncServiceCallback callback, void* userdata) {
            throwIfBad(UA_Client_Subscriptions_create_async(
                opcua::detail::getHandle(connection),
                asNative(request),
                context,
                detail::SubscriptionContext::statusChangeCallbackNative,
                detail::SubscriptionContext::deleteCallbackNative,
                callback,
                userdata,
                nullptr
            ));
        },
        detail::HookToken(
            [&, context = std::move(context)](const CreateSubscriptionResponse& response) mutable {
                if (detail::getServiceResult(response).isGood()) {
                    detail::storeSubscriptionContext(
                        connection, response.getSubscriptionId(), std::move(context)
                    );
                }
            },
            std::forward<CompletionToken>(token)
        )
    );
}

/// @overload
template <typename CompletionToken>
auto createSubscriptionAsync(
    Client& connection,
    const SubscriptionParameters& parameters,
    bool publishingEnabled,
    StatusChangeNotificationCallback statusChangeCallback,
    DeleteSubscriptionCallback deleteCallback,
    CompletionToken&& token
) {
    const auto request = detail::createCreateSubscriptionRequest(parameters, publishingEnabled);
    return createSubscriptionAsync(
        connection,
        asWrapper<CreateSubscriptionRequest>(request),
        std::move(statusChangeCallback),
        std::move(deleteCallback),
        std::forward<CompletionToken>(token)
    );
}
#endif

/**
 * @}
 * @defgroup ModifySubscription ModifySubscription service
 * Modify subscriptions.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.14.3
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

/// @overload
inline ModifySubscriptionResponse modifySubscription(
    Client& connection, IntegerId subscriptionId, const SubscriptionParameters& parameters
) noexcept {
    const auto request = detail::createModifySubscriptionRequest(subscriptionId, parameters);
    return modifySubscription(connection, asWrapper<ModifySubscriptionRequest>(request));
}

#if UAPP_HAS_ASYNC_SUBSCRIPTIONS
/**
 * @copydoc modifySubscription(Client&, const ModifySubscriptionRequest&)
 * @param token @completiontoken{void(ModifySubscriptionResponse&)}
 * @return @asyncresult{ModifySubscriptionResponse}
 */
template <typename CompletionToken>
auto modifySubscriptionAsync(
    Client& connection, const ModifySubscriptionRequest& request, CompletionToken&& token
) {
    return detail::AsyncServiceAdapter<ModifySubscriptionResponse>::initiate(
        connection,
        [&](UA_ClientAsyncServiceCallback callback, void* userdata) {
            throwIfBad(UA_Client_Subscriptions_modify_async(
                opcua::detail::getHandle(connection), asNative(request), callback, userdata, nullptr
            ));
        },
        std::forward<CompletionToken>(token)
    );
}

/// @overload
template <typename CompletionToken>
auto modifySubscriptionAsync(
    Client& connection,
    IntegerId subscriptionId,
    const SubscriptionParameters& parameters,
    CompletionToken&& token
) {
    const auto request = detail::createModifySubscriptionRequest(subscriptionId, parameters);
    return modifySubscriptionAsync(
        connection,
        asWrapper<ModifySubscriptionRequest>(request),
        std::forward<CompletionToken>(token)
    );
}
#endif

/**
 * @}
 * @defgroup SetPublishingMode SetPublishingMode service
 * Enable/disable sending of notifications on subscriptions.
 * Disable publishing of NotificationMessages of the subscription doesn't discontinue the sending
 * of keep-alive messages, nor change the monitoring mode.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.14.4
 * @{
 */

/**
 * Enable/disable publishing of notification messages of subscriptions.
 * @param connection Instance of type Client
 * @param request Set publishing mode request
 */
SetPublishingModeResponse setPublishingMode(
    Client& connection, const SetPublishingModeRequest& request
) noexcept;

/**
 * @copydoc setPublishingMode(Client&, const SetPublishingModeRequest&)
 * @param token @completiontoken{void(SetPublishingModeResponse&)}
 * @return @asyncresult{SetPublishingModeResponse}
 */
template <typename CompletionToken>
auto setPublishingModeAsync(
    Client& connection, const SetPublishingModeRequest& request, CompletionToken&& token
) {
    return detail::sendRequestAsync<SetPublishingModeRequest, SetPublishingModeResponse>(
        connection, request, std::forward<CompletionToken>(token)
    );
}

/**
 * Enable/disable publishing of notification messages of a single subscription.
 * @param connection Instance of type Client
 * @param subscriptionId Identifier of the subscription returned by @ref createSubscription
 * @param publishing Enable/disable publishing
 */
inline StatusCode setPublishingMode(
    Client& connection, IntegerId subscriptionId, bool publishing
) noexcept {
    const auto request = detail::createSetPublishingModeRequest(publishing, {&subscriptionId, 1});
    return detail::getSingleStatus(
        setPublishingMode(connection, asWrapper<SetPublishingModeRequest>(request))
    );
}

/**
 * @copydoc setPublishingMode(Client&, IntegerId, bool)
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 */
template <typename CompletionToken>
auto setPublishingModeAsync(
    Client& connection, IntegerId subscriptionId, bool publishing, CompletionToken&& token
) {
    const auto request = detail::createSetPublishingModeRequest(publishing, {&subscriptionId, 1});
    return setPublishingModeAsync(
        connection,
        asWrapper<SetPublishingModeRequest>(request),
        detail::TransformToken(
            detail::getSingleStatus<UA_SetPublishingModeResponse>,
            std::forward<CompletionToken>(token)
        )
    );
}

/**
 * @}
 * @defgroup DeleteSubscriptions DeleteSubscriptions service
 * Delete subscriptions.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.14.8
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

#if UAPP_HAS_ASYNC_SUBSCRIPTIONS
/**
 * @copydoc deleteSubscriptions
 * @param token @completiontoken{void(DeleteSubscriptionsResponse&)}
 * @return @asyncresult{DeleteSubscriptionsResponse}
 */
template <typename CompletionToken>
auto deleteSubscriptionsAsync(
    Client& connection, const DeleteSubscriptionsRequest& request, CompletionToken&& token
) {
    return detail::AsyncServiceAdapter<DeleteSubscriptionsResponse>::initiate(
        connection,
        [&](UA_ClientAsyncServiceCallback callback, void* userdata) {
            throwIfBad(UA_Client_Subscriptions_delete_async(
                opcua::detail::getHandle(connection), asNative(request), callback, userdata, nullptr
            ));
        },
        std::forward<CompletionToken>(token)
    );
}
#endif

/**
 * Delete a single subscription.
 * @param connection Instance of type Client
 * @param subscriptionId Identifier of the subscription returned by @ref createSubscription
 */
inline StatusCode deleteSubscription(Client& connection, IntegerId subscriptionId) noexcept {
    const auto request = detail::createDeleteSubscriptionsRequest(subscriptionId);
    return detail::getSingleStatus(
        deleteSubscriptions(connection, asWrapper<DeleteSubscriptionsRequest>(request))
    );
}

#if UAPP_HAS_ASYNC_SUBSCRIPTIONS
/**
 * @copydoc deleteSubscription
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 */
template <typename CompletionToken>
auto deleteSubscriptionAsync(
    Client& connection, IntegerId subscriptionId, CompletionToken&& token
) {
    const auto request = detail::createDeleteSubscriptionsRequest(subscriptionId);
    return deleteSubscriptionsAsync(
        connection,
        asWrapper<DeleteSubscriptionsRequest>(request),
        detail::TransformToken{
            detail::getSingleStatus<UA_DeleteSubscriptionsResponse>,
            std::forward<CompletionToken>(token)
        }
    );
}
#endif

/**
 * @}
 * @}
 */

}  // namespace opcua::services

#endif
