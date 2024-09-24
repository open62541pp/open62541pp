#include <doctest/doctest.h>

#include "open62541pp/Config.h"
#include "open62541pp/services/Subscription.h"

#include "helper/server_client_setup.h"

using namespace opcua;

#ifdef UA_ENABLE_SUBSCRIPTIONS
TEST_CASE("Subscription service set (client)") {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& client = setup.client;

    services::SubscriptionParameters parameters{};

    SUBCASE("createSubscription") {
        const auto subId = services::createSubscription(client, parameters).value();
        CAPTURE(subId);
    }

    SUBCASE("modifySubscription") {
        const auto subId = services::createSubscription(client, parameters).value();

        parameters.priority = 1;
        CHECK(services::modifySubscription(client, subId, parameters));
        CHECK(
            services::modifySubscription(client, subId + 1, parameters).code() ==
            UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID
        );
    }

    SUBCASE("setPublishingMode") {
        const auto subId = services::createSubscription(client, parameters).value();
        CHECK(services::setPublishingMode(client, subId, false));
    }

    SUBCASE("deleteSubscription") {
        const auto subId = services::createSubscription(client, parameters).value();
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
            services::createSubscription(client, parameters, true, {}, deleteCallback).value();

        CHECK(services::deleteSubscription(client, subId));
        CHECK(deleted == true);
    }
}
#endif
