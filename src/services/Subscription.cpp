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

Result<uint32_t> createSubscription(
    Client& connection,
    SubscriptionParameters& parameters,
    bool publishingEnabled,
    DeleteSubscriptionCallback deleteCallback
) {
    auto context = std::make_unique<detail::SubscriptionContext>();
    context->catcher = &opcua::detail::getContext(connection).exceptionCatcher;
    context->deleteCallback = std::move(deleteCallback);

    const CreateSubscriptionResponse response = UA_Client_Subscriptions_create(
        connection.handle(),
        detail::createCreateSubscriptionRequest(parameters, publishingEnabled),
        context.get(),
        nullptr,  // statusChangeCallback
        context->deleteCallbackNative
    );
    if (StatusCode serviceResult = detail::getServiceResult(response); serviceResult.isBad()) {
        return BadResult(serviceResult);
    }
    detail::reviseSubscriptionParameters(parameters, asNative(response));

    const auto subscriptionId = response.getSubscriptionId();
    opcua::detail::getContext(connection).subscriptions.insert(subscriptionId, std::move(context));
    return subscriptionId;
}

Result<void> modifySubscription(
    Client& connection, uint32_t subscriptionId, SubscriptionParameters& parameters
) noexcept {
    const ModifySubscriptionResponse response = UA_Client_Subscriptions_modify(
        connection.handle(), detail::createModifySubscriptionRequest(subscriptionId, parameters)
    );
    if (StatusCode serviceResult = detail::getServiceResult(response); serviceResult.isBad()) {
        return BadResult(serviceResult);
    }
    detail::reviseSubscriptionParameters(parameters, asNative(response));
    return {};
}

Result<void> setPublishingMode(
    Client& connection, uint32_t subscriptionId, bool publishing
) noexcept {
    return detail::sendRequest<UA_SetPublishingModeRequest, UA_SetPublishingModeResponse>(
        connection,
        detail::createSetPublishingModeRequest({&subscriptionId, 1}, publishing),
        [](UA_SetPublishingModeResponse& response) {
            return detail::getSingleResult(response).andThen(detail::asResult);
        },
        detail::SyncOperation{}
    );
}

Result<void> deleteSubscription(Client& connection, uint32_t subscriptionId) noexcept {
    return detail::asResult(
        UA_Client_Subscriptions_deleteSingle(connection.handle(), subscriptionId)
    );
}

}  // namespace opcua::services

#endif
