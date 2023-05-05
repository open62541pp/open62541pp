#include "open62541pp/services/Subscription.h"

#include <utility>  // move

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"

#include "../ClientContext.h"
#include "../ServerContext.h"
#include "../open62541_impl.h"

namespace opcua::services {

static void deleteSubscriptionCallback(
    [[maybe_unused]] UA_Client* client, uint32_t subId, void* subContext
) {
    if (subContext == nullptr) {
        return;
    }
    auto* subscription = static_cast<ClientContext::Subscription*>(subContext);
    if (subscription->deleteCallback) {
        subscription->deleteCallback(subId);
    }
    subscription->deleted = true;
}

template <>
uint32_t createSubscription<Server>(
    [[maybe_unused]] Server& server,
    [[maybe_unused]] SubscriptionParameters& parameters,
    [[maybe_unused]] bool publishingEnabled,
    DeleteSubscriptionCallback deleteCallback
) {
    server.getContext().addSubscription(std::move(deleteCallback));
    return 0U;
}

template <>
uint32_t createSubscription<Client>(
    Client& client,
    SubscriptionParameters& parameters,
    bool publishingEnabled,
    DeleteSubscriptionCallback deleteCallback
) {
    UA_CreateSubscriptionRequest request{};
    request.requestedPublishingInterval = parameters.publishingInterval;
    request.requestedLifetimeCount = parameters.lifetimeCount;
    request.requestedMaxKeepAliveCount = parameters.maxKeepAliveCount;
    request.maxNotificationsPerPublish = parameters.maxNotificationsPerPublish;
    request.publishingEnabled = publishingEnabled;
    request.priority = parameters.priority;

    auto subscriptionContext = std::make_unique<ClientContext::Subscription>();
    subscriptionContext->deleteCallback = std::move(deleteCallback);

    using Response =
        TypeWrapper<UA_CreateSubscriptionResponse, UA_TYPES_CREATESUBSCRIPTIONRESPONSE>;
    const Response response = UA_Client_Subscriptions_create(
        client.handle(),
        request,
        subscriptionContext.get(),
        nullptr,  // statusChangeCallback
        deleteSubscriptionCallback
    );
    detail::throwOnBadStatus(response->responseHeader.serviceResult);

    client.getContext().addSubscription(std::move(subscriptionContext));

    // update revised parameters
    parameters.publishingInterval = response->revisedPublishingInterval;
    parameters.lifetimeCount = response->revisedLifetimeCount;
    parameters.maxKeepAliveCount = response->revisedMaxKeepAliveCount;

    return response->subscriptionId;
}

template <>
void modifySubscription<Server>(
    [[maybe_unused]] Server& server,
    uint32_t subscriptionId,
    [[maybe_unused]] SubscriptionParameters& parameters
) {
    if (subscriptionId != 0U) {
        throw BadStatus(UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID);
    }
}

template <>
void modifySubscription<Client>(
    Client& client, uint32_t subscriptionId, SubscriptionParameters& parameters
) {
    UA_ModifySubscriptionRequest request{};
    request.subscriptionId = subscriptionId;
    request.requestedPublishingInterval = parameters.publishingInterval;
    request.requestedLifetimeCount = parameters.lifetimeCount;
    request.requestedMaxKeepAliveCount = parameters.maxKeepAliveCount;
    request.maxNotificationsPerPublish = parameters.maxNotificationsPerPublish;
    request.priority = parameters.priority;

    using Response =
        TypeWrapper<UA_ModifySubscriptionResponse, UA_TYPES_MODIFYSUBSCRIPTIONRESPONSE>;
    const Response response = UA_Client_Subscriptions_modify(client.handle(), request);
    detail::throwOnBadStatus(response->responseHeader.serviceResult);

    // update revised parameters
    parameters.publishingInterval = response->revisedPublishingInterval;
    parameters.lifetimeCount = response->revisedLifetimeCount;
    parameters.maxKeepAliveCount = response->revisedMaxKeepAliveCount;
}

template <>
void setPublishingMode<Server>(
    [[maybe_unused]] Server& server, uint32_t subscriptionId, [[maybe_unused]] bool publishing
) {
    if (subscriptionId != 0U) {
        throw BadStatus(UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID);
    }
}

template <>
void setPublishingMode<Client>(Client& client, uint32_t subscriptionId, bool publishing) {
    UA_SetPublishingModeRequest request{};
    request.publishingEnabled = publishing;
    request.subscriptionIdsSize = 1;
    request.subscriptionIds = &subscriptionId;

    using Response = TypeWrapper<UA_SetPublishingModeResponse, UA_TYPES_SETPUBLISHINGMODERESPONSE>;
    const Response response = UA_Client_Subscriptions_setPublishingMode(client.handle(), request);
    detail::throwOnBadStatus(response->responseHeader.serviceResult);

    if (response->resultsSize != 1) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
    detail::throwOnBadStatus(*response->results);
}

template <>
void deleteSubscription<Server>([[maybe_unused]] Server& server, uint32_t subscriptionId) {
    if (subscriptionId != 0U) {
        throw BadStatus(UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID);
    }
    server.getContext().deleteSubscription();
}

template <>
void deleteSubscription<Client>(Client& client, uint32_t subscriptionId) {
    const auto status = UA_Client_Subscriptions_deleteSingle(client.handle(), subscriptionId);
    detail::throwOnBadStatus(status);
}

}  // namespace opcua::services
