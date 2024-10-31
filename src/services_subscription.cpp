#include "open62541pp/services/subscription.hpp"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include <memory>
#include <utility>  // move

#include "open62541pp/client.hpp"
#include "open62541pp/detail/client_context.hpp"
#include "open62541pp/detail/open62541/client.h"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/services/detail/client_services.hpp"
#include "open62541pp/services/detail/response_handling.hpp"
#include "open62541pp/services/detail/subscription_context.hpp"
#include "open62541pp/types_composed.hpp"
#include "open62541pp/wrapper.hpp"  // asNative

namespace opcua::services {

CreateSubscriptionResponse createSubscription(
    Client& connection,
    const CreateSubscriptionRequest& request,
    StatusChangeNotificationCallback statusChangeCallback,
    DeleteSubscriptionCallback deleteCallback
) {
    auto context = std::make_unique<detail::SubscriptionContext>();
    context->catcher = &opcua::detail::getContext(connection).exceptionCatcher;
    context->statusChangeCallback = std::move(statusChangeCallback);
    context->deleteCallback = std::move(deleteCallback);

    CreateSubscriptionResponse response = UA_Client_Subscriptions_create(
        connection.handle(),
        request,
        context.get(),
        context->statusChangeCallbackNative,
        context->deleteCallbackNative
    );
    if (detail::getServiceResult(response).isGood()) {
        opcua::detail::getContext(connection)
            .subscriptions.insert(response.getSubscriptionId(), std::move(context));
    }
    return response;
}

CreateSubscriptionResponse createSubscription(
    Client& connection,
    const SubscriptionParameters& parameters,
    bool publishingEnabled,
    StatusChangeNotificationCallback statusChangeCallback,
    DeleteSubscriptionCallback deleteCallback
) noexcept {
    UA_CreateSubscriptionRequest request{};
    request.requestedPublishingInterval = parameters.publishingInterval;
    request.requestedLifetimeCount = parameters.lifetimeCount;
    request.requestedMaxKeepAliveCount = parameters.maxKeepAliveCount;
    request.maxNotificationsPerPublish = parameters.maxNotificationsPerPublish;
    request.publishingEnabled = publishingEnabled;
    request.priority = parameters.priority;
    return createSubscription(
        connection,
        asWrapper<CreateSubscriptionRequest>(request),
        std::move(statusChangeCallback),
        std::move(deleteCallback)
    );
}

ModifySubscriptionResponse modifySubscription(
    Client& connection, const ModifySubscriptionRequest& request
) noexcept {
    return UA_Client_Subscriptions_modify(connection.handle(), request);
}

ModifySubscriptionResponse modifySubscription(
    Client& connection, uint32_t subscriptionId, const SubscriptionParameters& parameters
) noexcept {
    UA_ModifySubscriptionRequest request{};
    request.subscriptionId = subscriptionId;
    request.requestedPublishingInterval = parameters.publishingInterval;
    request.requestedLifetimeCount = parameters.lifetimeCount;
    request.requestedMaxKeepAliveCount = parameters.maxKeepAliveCount;
    request.maxNotificationsPerPublish = parameters.maxNotificationsPerPublish;
    request.priority = parameters.priority;
    return modifySubscription(connection, asWrapper<ModifySubscriptionRequest>(request));
}

SetPublishingModeResponse setPublishingMode(
    Client& connection, const SetPublishingModeRequest& request
) noexcept {
    return UA_Client_Subscriptions_setPublishingMode(connection.handle(), request);
}

StatusCode setPublishingMode(
    Client& connection, uint32_t subscriptionId, bool publishing
) noexcept {
    UA_SetPublishingModeRequest request{};
    request.publishingEnabled = publishing;
    request.subscriptionIdsSize = 1;
    request.subscriptionIds = &subscriptionId;
    const auto response = setPublishingMode(
        connection, asWrapper<SetPublishingModeRequest>(request)
    );
    return detail::getSingleStatus(response);
}

DeleteSubscriptionsResponse deleteSubscriptions(
    Client& connection, const DeleteSubscriptionsRequest& request
) noexcept {
    return UA_Client_Subscriptions_delete(connection.handle(), request);
}

StatusCode deleteSubscription(Client& connection, uint32_t subscriptionId) noexcept {
    UA_DeleteSubscriptionsRequest request{};
    request.subscriptionIdsSize = 1;
    request.subscriptionIds = &subscriptionId;
    const auto response = deleteSubscriptions(
        connection, asWrapper<DeleteSubscriptionsRequest>(request)
    );
    return detail::getSingleStatus(asNative(response));
}

}  // namespace opcua::services

#endif
