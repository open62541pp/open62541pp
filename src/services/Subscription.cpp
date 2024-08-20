#include "open62541pp/services/Subscription.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include <memory>
#include <utility>  // move

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/ClientContext.h"
#include "open62541pp/detail/open62541/client.h"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/services/detail/ClientService.h"
#include "open62541pp/services/detail/RequestHandling.h"
#include "open62541pp/services/detail/ResponseHandling.h"
#include "open62541pp/services/detail/SubscriptionContext.h"
#include "open62541pp/types/Composed.h"

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

Result<uint32_t> createSubscription(
    Client& connection,
    SubscriptionParameters& parameters,
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
    const auto response = createSubscription(
        connection,
        asWrapper<CreateSubscriptionRequest>(request),
        std::move(statusChangeCallback),
        std::move(deleteCallback)
    );
    if (StatusCode serviceResult = detail::getServiceResult(response); serviceResult.isBad()) {
        return BadResult(serviceResult);
    }
    detail::reviseSubscriptionParameters(parameters, asNative(response));
    return response.getSubscriptionId();
}

ModifySubscriptionResponse modifySubscription(
    Client& connection, const ModifySubscriptionRequest& request
) noexcept {
    return UA_Client_Subscriptions_modify(connection.handle(), request);
}

Result<void> modifySubscription(
    Client& connection, uint32_t subscriptionId, SubscriptionParameters& parameters
) noexcept {
    UA_ModifySubscriptionRequest request{};
    request.subscriptionId = subscriptionId;
    request.requestedPublishingInterval = parameters.publishingInterval;
    request.requestedLifetimeCount = parameters.lifetimeCount;
    request.requestedMaxKeepAliveCount = parameters.maxKeepAliveCount;
    request.maxNotificationsPerPublish = parameters.maxNotificationsPerPublish;
    request.priority = parameters.priority;
    const ModifySubscriptionResponse response = UA_Client_Subscriptions_modify(
        connection.handle(), request
    );
    if (StatusCode serviceResult = detail::getServiceResult(response); serviceResult.isBad()) {
        return BadResult(serviceResult);
    }
    detail::reviseSubscriptionParameters(parameters, asNative(response));
    return {};
}

SetPublishingModeResponse setPublishingMode(
    Client& connection, const SetPublishingModeRequest& request
) noexcept {
    return detail::sendRequest<UA_SetPublishingModeRequest, UA_SetPublishingModeResponse>(
        connection, request, detail::Wrap<SetPublishingModeResponse>{}, detail::SyncOperation{}
    );
}

Result<void> setPublishingMode(
    Client& connection, uint32_t subscriptionId, bool publishing
) noexcept {
    UA_SetPublishingModeRequest request{};
    request.publishingEnabled = publishing;
    request.subscriptionIdsSize = 1;
    request.subscriptionIds = &subscriptionId;
    return detail::sendRequest<UA_SetPublishingModeRequest, UA_SetPublishingModeResponse>(
        connection,
        request,
        [](UA_SetPublishingModeResponse& response) {
            return detail::getSingleResult(response).andThen(detail::toResult);
        },
        detail::SyncOperation{}
    );
}

DeleteSubscriptionsResponse deleteSubscriptions(
    Client& connection, const DeleteSubscriptionsRequest& request
) noexcept {
    return UA_Client_Subscriptions_delete(connection.handle(), request);
}

Result<void> deleteSubscription(Client& connection, uint32_t subscriptionId) noexcept {
    UA_DeleteSubscriptionsRequest request{};
    request.subscriptionIdsSize = 1;
    request.subscriptionIds = &subscriptionId;
    const DeleteSubscriptionsResponse response = UA_Client_Subscriptions_delete(
        connection.handle(), request
    );
    return detail::getSingleResult(asNative(response)).andThen(detail::toResult);
}

}  // namespace opcua::services

#endif
