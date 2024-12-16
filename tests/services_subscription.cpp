#include <doctest/doctest.h>

#include "open62541pp/config.hpp"
#include "open62541pp/detail/client_context.hpp"
#include "open62541pp/services/subscription.hpp"

#include "helper/server_client_setup.hpp"

using namespace opcua;

#ifdef UA_ENABLE_SUBSCRIPTIONS
TEST_CASE_TEMPLATE("Subscription service set", T, Client, Async<Client>) {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& connection = setup.instance<T>();

    services::SubscriptionParameters parameters{};

    SUBCASE("createSubscription") {
        CreateSubscriptionResponse response;
        if constexpr (isAsync<T> && UAPP_HAS_ASYNC_SUBSCRIPTIONS) {
#if UAPP_HAS_ASYNC_SUBSCRIPTIONS
            auto future = services::createSubscriptionAsync(
                connection, parameters, true, {}, {}, useFuture
            );
            setup.client.runIterate();
            response = future.get();
#endif
        } else {
            response = services::createSubscription(connection, parameters, true, {}, {});
        }
        CHECK(response.responseHeader().serviceResult().isGood());
        CAPTURE(response.subscriptionId());
        CHECK(detail::getContext(connection).subscriptions.contains(response.subscriptionId()));
    }

    SUBCASE("modifySubscription") {
        const auto subId =
            services::createSubscription(connection, parameters, true, {}, {}).subscriptionId();

        parameters.priority = 1;
        ModifySubscriptionResponse response;
        if constexpr (isAsync<T> && UAPP_HAS_ASYNC_SUBSCRIPTIONS) {
#if UAPP_HAS_ASYNC_SUBSCRIPTIONS
            auto future = services::modifySubscriptionAsync(
                connection, subId, parameters, useFuture
            );
            setup.client.runIterate();
            response = future.get();
#endif
        } else {
            response = services::modifySubscription(connection, subId, parameters);
        }
        CHECK(response.responseHeader().serviceResult().isGood());

        SUBCASE("Invalid subscription id") {
            CHECK(
                services::modifySubscription(connection, subId + 1, parameters)
                    .responseHeader()
                    .serviceResult() == UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID
            );
        }
    }

    SUBCASE("setPublishingMode") {
        const auto subId =
            services::createSubscription(connection, parameters, true, {}, {}).subscriptionId();

        if constexpr (isAsync<T>) {
            auto future = services::setPublishingModeAsync(connection, subId, false, useFuture);
            setup.client.runIterate();
            CHECK(future.get().isGood());
        } else {
            CHECK(services::setPublishingMode(connection, subId, false).isGood());
        }
    }

    SUBCASE("deleteSubscription") {
        const auto subId =
            services::createSubscription(connection, parameters, true, {}, {}).subscriptionId();

        if constexpr (isAsync<T>) {
#if UAPP_HAS_ASYNC_SUBSCRIPTIONS && UAPP_OPEN62541_VER_LE(1, 3)
            auto future = services::deleteSubscriptionAsync(connection, subId, useFuture);
            setup.client.runIterate();
            // TODO: multiple calls required by v1.4
            CHECK(future.get().isGood());
#endif
        } else {
            CHECK(services::deleteSubscription(connection, subId).isGood());
        }

        SUBCASE("Invalid subscription id") {
            CHECK(
                services::deleteSubscription(connection, subId + 1) ==
                UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID
            );
        }
    }

    SUBCASE("deleteSubscription with callback") {
        bool deleted = false;
        const auto deleteCallback = [&](IntegerId) { deleted = true; };
        const auto subId =
            services::createSubscription(connection, parameters, true, {}, deleteCallback)
                .subscriptionId();

        CHECK(services::deleteSubscription(connection, subId).isGood());
        CHECK(deleted == true);
    }
}
#endif
