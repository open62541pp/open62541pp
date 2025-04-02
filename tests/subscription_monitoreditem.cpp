#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "open62541pp/client.hpp"
#include "open62541pp/config.hpp"
#include "open62541pp/monitoreditem.hpp"
#include "open62541pp/server.hpp"
#include "open62541pp/subscription.hpp"
#include "open62541pp/ua/types.hpp"

#include "helper/server_client_setup.hpp"
#include "helper/server_runner.hpp"

using namespace opcua;

#ifdef UA_ENABLE_SUBSCRIPTIONS
TEST_CASE("Subscription & MonitoredItem (server)") {
    Server server;

    SECTION("Create Subscription with arbitrary id") {
        CHECK(Subscription(server, 11U).connection() == server);
        CHECK(Subscription(server, 11U).subscriptionId() == 0U);
    }

    SECTION("Create MonitoredItem with arbitrary ids") {
        CHECK(MonitoredItem(server, 11U, 22U).connection() == server);
        CHECK(MonitoredItem(server, 11U, 22U).subscriptionId() == 0U);
        CHECK(MonitoredItem(server, 11U, 22U).monitoredItemId() == 22U);
    }

    SECTION("Create & delete subscription") {
        Subscription sub(server);
        CHECK(sub.subscriptionId() == 0U);
        CHECK(sub.monitoredItems().empty());

        MonitoringParametersEx monitoringParameters{};
        monitoringParameters.samplingInterval = 0.0;  // = fastest practical

        size_t notificationCount = 0;
        auto mon = sub.subscribeDataChange(
            VariableId::Server_ServerStatus_CurrentTime,
            AttributeId::Value,
            MonitoringMode::Reporting,
            monitoringParameters,
            [&](IntegerId, IntegerId, const DataValue&) { notificationCount++; }
        );
        CHECK(sub.monitoredItems().size() == 1);

        CHECK(runIterateUntil(server, [&] { return notificationCount > 0; }));

        mon.deleteMonitoredItem();
        CHECK(sub.monitoredItems().empty());
    }
}

TEST_CASE("Subscription & MonitoredItem (client)") {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& client = setup.client;

    SECTION("Create Subscription with arbitrary id") {
        CHECK(Subscription(client, 11U).connection() == client);
        CHECK(Subscription(client, 11U).subscriptionId() == 11U);
    }

    SECTION("Create MonitoredItem with arbitrary ids") {
        CHECK(MonitoredItem(client, 11U, 22U).connection() == client);
        CHECK(MonitoredItem(client, 11U, 22U).subscriptionId() == 11U);
        CHECK(MonitoredItem(client, 11U, 22U).monitoredItemId() == 22U);
    }

    SECTION("Create & delete subscription") {
        CHECK(client.subscriptions().empty());

        const SubscriptionParameters parameters{};
        Subscription sub(client, parameters);
        CHECK(sub.subscriptionId() > 0U);

        CHECK(client.subscriptions().size() == 1);
        CHECK(client.subscriptions().at(0) == sub);

        CHECK(sub.monitoredItems().empty());

        sub.deleteSubscription();
        CHECK(client.subscriptions().empty());
        CHECK_THROWS_WITH(sub.deleteSubscription(), "BadSubscriptionIdInvalid");
    }

    SECTION("Modify subscription") {
        SubscriptionParameters parameters{};
        Subscription sub(client, parameters);

        sub.setPublishingMode(false);

        parameters.priority = 10;
        sub.setSubscriptionParameters(parameters);
    }

    SECTION("Monitor data change") {
        const SubscriptionParameters subscriptionParameters{};
        MonitoringParametersEx monitoringParameters{};

        Subscription sub(client, subscriptionParameters);
        sub.setPublishingMode(false);  // enable later

        size_t notificationCount = 0;
        auto mon = sub.subscribeDataChange(
            VariableId::Server_ServerStatus_CurrentTime,
            AttributeId::Value,
            MonitoringMode::Sampling,  // won't trigger notifications
            monitoringParameters,
            [&](IntegerId, IntegerId, const DataValue&) { notificationCount++; }
        );

        CHECK(sub.monitoredItems().size() == 1);
        CHECK(sub.monitoredItems().at(0) == mon);

        client.runIterate();
        CHECK(notificationCount == 0);

        sub.setPublishingMode(true);  // monitoring mode is still sampling -> no notifications
        client.runIterate();
        CHECK(notificationCount == 0);

        mon.setMonitoringMode(MonitoringMode::Reporting);  // now we should get a notification
        CHECK(runIterateUntil(client, [&] { return notificationCount > 0; }));

        mon.deleteMonitoredItem();
        CHECK_THROWS_WITH(mon.deleteMonitoredItem(), "BadMonitoredItemIdInvalid");
    }

    SECTION("Monitor data change with multiple monitored items") {
        Subscription sub(client);

        IntegerId monId1 = 0;
        auto monItem1 = sub.subscribeDataChange(
            VariableId::Server_ServerStatus_CurrentTime,
            AttributeId::Value,
            [&](IntegerId, IntegerId monId, const DataValue&) { monId1 = monId; }
        );

        IntegerId monId2 = 0;
        auto monItem2 = sub.subscribeDataChange(
            VariableId::Server_ServerStatus_CurrentTime,
            AttributeId::Value,
            [&](IntegerId, IntegerId monId, const DataValue&) { monId2 = monId; }
        );

        client.runIterate();
        CHECK(monId1 != 0);
        CHECK(monId2 != 0);
        CHECK(monId2 != monId1);
        CHECK(monItem1.monitoredItemId() == monId1);
        CHECK(monItem2.monitoredItemId() == monId2);
    }

    SECTION("Modify monitored item") {
        Subscription sub(client);
        auto mon = sub.subscribeDataChange(
            VariableId::Server_ServerStatus_CurrentTime,
            AttributeId::Value,
            DataChangeNotificationCallback{}
        );

        mon.setMonitoringMode(MonitoringMode::Disabled);

        MonitoringParametersEx monitoringParameters{};
        monitoringParameters.samplingInterval = 0.0;  // = fastest practical rate
        mon.setMonitoringParameters(monitoringParameters);
    }

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
    SECTION("Monitor event") {
        Subscription sub(client);

        EventFilter eventFilter(
            {
                {ObjectTypeId::BaseEventType, {{0, "Time"}}, AttributeId::Value},
                {ObjectTypeId::BaseEventType, {{0, "Severity"}}, AttributeId::Value},
                {ObjectTypeId::BaseEventType, {{0, "Message"}}, AttributeId::Value},
            },
            {}  // where clause -> no filter
        );
        auto mon = sub.subscribeEvent(
            ObjectId::Server,
            eventFilter,
            [](IntegerId subId, IntegerId monId, const auto& eventFields) {
                CAPTURE(subId);
                CAPTURE(monId);
                CAPTURE(eventFields.size());
            }
        );

        CHECK(sub.monitoredItems().size() == 1);
        CHECK(sub.monitoredItems().at(0) == mon);
    }
#endif
}
#endif
