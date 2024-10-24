#include <doctest/doctest.h>

#include "open62541pp/config.hpp"
#include "open62541pp/services/subscription.hpp"

#include "helper/server_client_setup.hpp"

using namespace opcua;

#ifdef UA_ENABLE_SUBSCRIPTIONS
TEST_CASE("Subscription service set (client)") {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& client = setup.client;

    services::SubscriptionParameters parameters{};

    SUBCASE("createSubscription") {
        const auto response = services::createSubscription(client, parameters);
        CHECK(response.getResponseHeader().getServiceResult().isGood());
        CAPTURE(response.getSubscriptionId());
    }

    SUBCASE("modifySubscription") {
        const auto subId = services::createSubscription(client, parameters).getSubscriptionId();

        parameters.priority = 1;
        {
            const auto response = services::modifySubscription(client, subId, parameters);
            CHECK(response.getResponseHeader().getServiceResult().isGood());
        }
        {
            const auto response = services::modifySubscription(client, subId + 1, parameters);
            CHECK(
                response.getResponseHeader().getServiceResult() ==
                UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID
            );
        }
    }

    SUBCASE("setPublishingMode") {
        const auto subId = services::createSubscription(client, parameters).getSubscriptionId();
        CHECK(services::setPublishingMode(client, subId, false));
    }

    SUBCASE("deleteSubscription") {
        const auto subId = services::createSubscription(client, parameters).getSubscriptionId();
        CHECK(services::deleteSubscription(client, subId));
        CHECK(
            services::deleteSubscription(client, subId + 1).code() ==
            UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID
        );
    }

    SUBCASE("deleteSubscription with callback") {
        bool deleted = false;
        const auto deleteCallback = [&](uint32_t) { deleted = true; };
        const auto subId =
            services::createSubscription(client, parameters, true, {}, deleteCallback)
                .getSubscriptionId();

        CHECK(services::deleteSubscription(client, subId));
        CHECK(deleted == true);
    }
}
#endif
