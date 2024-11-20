#include "open62541pp/services/subscription.hpp"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include <memory>

#include "open62541pp/client.hpp"
#include "open62541pp/detail/client_context.hpp"
#include "open62541pp/detail/open62541/client.h"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/services/detail/response_handling.hpp"
#include "open62541pp/services/detail/subscription_context.hpp"
#include "open62541pp/ua/types.hpp"

namespace opcua::services {

namespace detail {

std::unique_ptr<SubscriptionContext> createSubscriptionContext(
    Client& connection,
    StatusChangeNotificationCallback&& statusChangeCallback,
    DeleteSubscriptionCallback&& deleteCallback
) {
    auto context = std::make_unique<SubscriptionContext>();
    context->catcher = &opcua::detail::getContext(connection).exceptionCatcher;
    context->statusChangeCallback = std::move(statusChangeCallback);
    context->deleteCallback = std::move(deleteCallback);
    return context;
}

void storeSubscriptionContext(
    Client& connection, IntegerId subscriptionId, std::unique_ptr<SubscriptionContext>&& context
) {
    opcua::detail::getContext(connection).subscriptions.insert(subscriptionId, std::move(context));
}

}  // namespace detail

CreateSubscriptionResponse createSubscription(
    Client& connection,
    const CreateSubscriptionRequest& request,
    StatusChangeNotificationCallback statusChangeCallback,
    DeleteSubscriptionCallback deleteCallback
) {
    auto context = detail::createSubscriptionContext(
        connection, std::move(statusChangeCallback), std::move(deleteCallback)
    );
    CreateSubscriptionResponse response = UA_Client_Subscriptions_create(
        connection.handle(),
        request,
        context.get(),
        detail::SubscriptionContext::statusChangeCallbackNative,
        detail::SubscriptionContext::deleteCallbackNative
    );
    if (detail::getServiceResult(response).isGood()) {
        detail::storeSubscriptionContext(
            connection, response.getSubscriptionId(), std::move(context)
        );
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
    const auto request = detail::createCreateSubscriptionRequest(parameters, publishingEnabled);
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

SetPublishingModeResponse setPublishingMode(
    Client& connection, const SetPublishingModeRequest& request
) noexcept {
    return UA_Client_Subscriptions_setPublishingMode(connection.handle(), request);
}

DeleteSubscriptionsResponse deleteSubscriptions(
    Client& connection, const DeleteSubscriptionsRequest& request
) noexcept {
    return UA_Client_Subscriptions_delete(connection.handle(), request);
}

}  // namespace opcua::services

#endif
