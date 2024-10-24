#include <chrono>
#include <thread>

#include <doctest/doctest.h>

#include "open62541pp/client.hpp"
#include "open62541pp/config.hpp"
#include "open62541pp/monitoreditem.hpp"
#include "open62541pp/server.hpp"
#include "open62541pp/subscription.hpp"
#include "open62541pp/types_composed.hpp"

#include "helper/server_client_setup.hpp"
#include "helper/server_runner.hpp"

using namespace opcua;

#ifdef UA_ENABLE_SUBSCRIPTIONS
TEST_CASE("Subscription & MonitoredItem (server)") {
    Server server;

    SUBCASE("Create Subscription with arbitrary id") {
        CHECK(Subscription(server, 11U).connection() == server);
        CHECK(Subscription(server, 11U).subscriptionId() == 0U);
    }

    SUBCASE("Create MonitoredItem with arbitrary ids") {
        CHECK(MonitoredItem(server, 11U, 22U).connection() == server);
        CHECK(MonitoredItem(server, 11U, 22U).subscriptionId() == 0U);
        CHECK(MonitoredItem(server, 11U, 22U).monitoredItemId() == 22U);
    }

    SUBCASE("Create & delete subscription") {
        auto sub = server.createSubscription();
        CHECK(sub.getMonitoredItems().empty());

        MonitoringParametersEx monitoringParameters{};
        monitoringParameters.samplingInterval = 0.0;  // = fastest practical

        size_t notificationCount = 0;
        auto mon = sub.subscribeDataChange(
            VariableId::Server_ServerStatus_CurrentTime,
            AttributeId::Value,
            MonitoringMode::Reporting,
            monitoringParameters,
            [&](uint32_t, uint32_t, const DataValue&) { notificationCount++; }
        );
        CHECK(sub.getMonitoredItems().size() == 1);

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        server.runIterate();
        CHECK(notificationCount > 0);

        mon.deleteMonitoredItem();
        CHECK(sub.getMonitoredItems().empty());
    }
}

TEST_CASE("Subscription & MonitoredItem (client)") {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& client = setup.client;

    SUBCASE("Create Subscription with arbitrary id") {
        CHECK(Subscription(client, 11U).connection() == client);
        CHECK(Subscription(client, 11U).subscriptionId() == 11U);
    }

    SUBCASE("Create MonitoredItem with arbitrary ids") {
        CHECK(MonitoredItem(client, 11U, 22U).connection() == client);
        CHECK(MonitoredItem(client, 11U, 22U).subscriptionId() == 11U);
        CHECK(MonitoredItem(client, 11U, 22U).monitoredItemId() == 22U);
    }

    SUBCASE("Create & delete subscription") {
        CHECK(client.getSubscriptions().empty());

        const SubscriptionParameters parameters{};
        auto sub = client.createSubscription(parameters);
        CAPTURE(sub.subscriptionId());

        CHECK(client.getSubscriptions().size() == 1);
        CHECK(client.getSubscriptions().at(0) == sub);

        CHECK(sub.getMonitoredItems().empty());

        sub.deleteSubscription();
        CHECK(client.getSubscriptions().empty());
        CHECK_THROWS_WITH(sub.deleteSubscription(), "BadSubscriptionIdInvalid");
    }

    SUBCASE("Modify subscription") {
        auto sub = client.createSubscription();
        sub.setPublishingMode(false);

        SubscriptionParameters parameters{};
        parameters.priority = 10;
        sub.setSubscriptionParameters(parameters);
    }

    SUBCASE("Monitor data change") {
        const SubscriptionParameters subscriptionParameters{};
        MonitoringParametersEx monitoringParameters{};

        auto sub = client.createSubscription(subscriptionParameters);
        sub.setPublishingMode(false);  // enable later

        size_t notificationCount = 0;
        auto mon = sub.subscribeDataChange(
            VariableId::Server_ServerStatus_CurrentTime,
            AttributeId::Value,
            MonitoringMode::Sampling,  // won't trigger notifications
            monitoringParameters,
            [&](uint32_t, uint32_t, const DataValue&) { notificationCount++; }
        );

        CHECK(sub.getMonitoredItems().size() == 1);
        CHECK(sub.getMonitoredItems().at(0) == mon);

        client.runIterate();
        CHECK(notificationCount == 0);

        sub.setPublishingMode(true);  // monitoring mode is still sampling -> no notifications
        client.runIterate();
        CHECK(notificationCount == 0);

        mon.setMonitoringMode(MonitoringMode::Reporting);  // now we should get a notification
        client.runIterate();
#if UAPP_OPEN62541_VER_LE(1, 3)
        // TODO: fails sporadically with v1.4, why?
        CHECK(notificationCount > 0);
#endif

        mon.deleteMonitoredItem();
        CHECK_THROWS_WITH(mon.deleteMonitoredItem(), "BadMonitoredItemIdInvalid");
    }

    SUBCASE("Monitor data change with multiple monitored items") {
        auto sub = client.createSubscription();

        uint32_t monId1 = 0;
        auto monItem1 = sub.subscribeDataChange(
            VariableId::Server_ServerStatus_CurrentTime,
            AttributeId::Value,
            [&](uint32_t, uint32_t monId, const DataValue&) { monId1 = monId; }
        );

        uint32_t monId2 = 0;
        auto monItem2 = sub.subscribeDataChange(
            VariableId::Server_ServerStatus_CurrentTime,
            AttributeId::Value,
            [&](uint32_t, uint32_t monId, const DataValue&) { monId2 = monId; }
        );

        client.runIterate();
        CHECK(monId1 != 0);
        CHECK(monId2 != 0);
        CHECK(monId2 != monId1);
        CHECK(monItem1.monitoredItemId() == monId1);
        CHECK(monItem2.monitoredItemId() == monId2);
    }

    SUBCASE("Modify monitored item") {
        auto sub = client.createSubscription();
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
    SUBCASE("Monitor event") {
        auto sub = client.createSubscription();

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
            [](uint32_t subId, uint32_t monId, const auto& eventFields) {
                CAPTURE(subId);
                CAPTURE(monId);
                CAPTURE(eventFields.size());
            }
        );

        CHECK(sub.getMonitoredItems().size() == 1);
        CHECK(sub.getMonitoredItems().at(0) == mon);
    }
#endif
}
#endif
