#include "open62541pp/services/Subscription.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include <memory>
#include <utility>  // move

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/open62541.h"

#include "../ClientContext.h"
#include "../open62541_impl.h"

namespace opcua::services {

static void deleteSubscriptionCallback(
    UA_Client* client, uint32_t subId, void* subContext
) noexcept {
    if (subContext != nullptr) {
        auto* subscription = static_cast<ClientContext::Subscription*>(subContext);
        auto& callback = subscription->deleteCallback;
        if (callback) {
            detail::invokeCatchIgnore([&] { callback(subId); });
        }
    }
    ClientContext& clientContext = getContext(client);
    clientContext.subscriptions.erase(subId);
}

uint32_t createSubscription(
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

    // update revised parameters
    parameters.publishingInterval = response->revisedPublishingInterval;
    parameters.lifetimeCount = response->revisedLifetimeCount;
    parameters.maxKeepAliveCount = response->revisedMaxKeepAliveCount;

    const auto subscriptionId = response->subscriptionId;
    client.getContext().subscriptions.insert_or_assign(
        subscriptionId, std::move(subscriptionContext)
    );

    return subscriptionId;
}

void modifySubscription(
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

void setPublishingMode(Client& client, uint32_t subscriptionId, bool publishing) {
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

void deleteSubscription(Client& client, uint32_t subscriptionId) {
    const auto status = UA_Client_Subscriptions_deleteSingle(client.handle(), subscriptionId);
    detail::throwOnBadStatus(status);
}

}  // namespace opcua::services

#endif
