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

namespace opcua::services {

uint32_t createSubscription(
    Client& connection,
    SubscriptionParameters& parameters,
    bool publishingEnabled,
    DeleteSubscriptionCallback deleteCallback
) {
    auto context = std::make_unique<detail::SubscriptionContext>();
    context->catcher = &opcua::detail::getContext(connection).exceptionCatcher;
    context->deleteCallback = std::move(deleteCallback);

    using Response =
        TypeWrapper<UA_CreateSubscriptionResponse, UA_TYPES_CREATESUBSCRIPTIONRESPONSE>;
    const Response response = UA_Client_Subscriptions_create(
        connection.handle(),
        detail::createCreateSubscriptionRequest(parameters, publishingEnabled),
        context.get(),
        nullptr,  // statusChangeCallback
        context->deleteCallbackNative
    );
    throwIfBad(response->responseHeader.serviceResult);
    detail::reviseSubscriptionParameters(parameters, asNative(response));

    const auto subscriptionId = response->subscriptionId;
    opcua::detail::getContext(connection).subscriptions.insert(subscriptionId, std::move(context));
    return subscriptionId;
}

void modifySubscription(
    Client& connection, uint32_t subscriptionId, SubscriptionParameters& parameters
) {
    using Response =
        TypeWrapper<UA_ModifySubscriptionResponse, UA_TYPES_MODIFYSUBSCRIPTIONRESPONSE>;
    const Response response = UA_Client_Subscriptions_modify(
        connection.handle(), detail::createModifySubscriptionRequest(subscriptionId, parameters)
    );
    throwIfBad(response->responseHeader.serviceResult);
    detail::reviseSubscriptionParameters(parameters, asNative(response));
}

void setPublishingMode(Client& connection, uint32_t subscriptionId, bool publishing) {
    detail::sendRequest<UA_SetPublishingModeRequest, UA_SetPublishingModeResponse>(
        connection,
        detail::createSetPublishingModeRequest({&subscriptionId, 1}, publishing),
        [](UA_SetPublishingModeResponse& response) {
            throwIfBad(detail::getSingleResult(response));
        },
        detail::SyncOperation{}
    );
}

void deleteSubscription(Client& connection, uint32_t subscriptionId) {
    const auto status = UA_Client_Subscriptions_deleteSingle(connection.handle(), subscriptionId);
    throwIfBad(status);
}

}  // namespace opcua::services

#endif
