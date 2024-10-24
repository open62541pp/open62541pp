#include <chrono>
#include <thread>

#include <doctest/doctest.h>

#include "open62541pp/config.hpp"
#include "open62541pp/event.hpp"
#include "open62541pp/nodeids.hpp"
#include "open62541pp/services/attribute_highlevel.hpp"  // writeValue
#include "open62541pp/services/monitoreditem.hpp"
#include "open62541pp/services/nodemanagement.hpp"  // addVariable
#include "open62541pp/services/subscription.hpp"
#include "open62541pp/types.hpp"

#include "helper/server_client_setup.hpp"

using namespace opcua;

#ifdef UA_ENABLE_SUBSCRIPTIONS
TEST_CASE("MonitoredItem service set (client)") {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& server = setup.server;
    auto& client = setup.client;

    // add variable node to test data change notifications
    const NodeId id{1, 1000};
    services::addVariable(server, {0, UA_NS0ID_OBJECTSFOLDER}, id, "Variable").value();

    const services::SubscriptionParameters subscriptionParameters{};
    services::MonitoringParametersEx monitoringParameters{};
    monitoringParameters.samplingInterval = 0.0;  // fastest

    SUBCASE("createMonitoredItemDataChange without subscription") {
        CHECK_FALSE(services::createMonitoredItemDataChange(
            client,
            11U,  // random subId
            {id, AttributeId::Value},
            MonitoringMode::Reporting,
            monitoringParameters,
            {}
        ));
    }

    const auto subId =
        services::createSubscription(client, subscriptionParameters).getSubscriptionId();
    CAPTURE(subId);

    SUBCASE("createMonitoredItemDataChange") {
        size_t notificationCount = 0;
        DataValue changedValue;
        const auto callback = [&](uint32_t, uint32_t, const DataValue& value) {
            notificationCount++;
            changedValue = value;
        };

        SUBCASE("Raw") {
            const CreateMonitoredItemsRequest request(
                {},
                subId,
                TimestampsToReturn::Both,
                {
                    MonitoredItemCreateRequest({id, AttributeId::Value}, MonitoringMode::Reporting),
                }
            );
            const auto response = services::createMonitoredItemsDataChange(
                client, request, callback
            );
            CHECK(response.getResponseHeader().getServiceResult().isGood());
        }
        SUBCASE("Single") {
            const auto monId =
                services::createMonitoredItemDataChange(
                    client,
                    subId,
                    {id, AttributeId::Value},
                    MonitoringMode::Reporting,
                    monitoringParameters,
                    callback
                )
                    .value();
            CAPTURE(monId);
        }

        services::writeValue(server, id, Variant::fromScalar(11.11)).value();
        client.runIterate();
        CHECK(notificationCount > 0);
        CHECK(changedValue.getValue().getScalar<double>() == 11.11);
    }

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
    SUBCASE("createMonitoredItemEvent") {
        const EventFilter eventFilter(
            // select clause
            {
                {ObjectTypeId::BaseEventType, {{0, "Time"}}, AttributeId::Value},
                {ObjectTypeId::BaseEventType, {{0, "Severity"}}, AttributeId::Value},
                {ObjectTypeId::BaseEventType, {{0, "Message"}}, AttributeId::Value},
            },
            // where clause
            {}
        );
        monitoringParameters.filter = ExtensionObject::fromDecodedCopy(eventFilter);

        size_t notificationCount = 0;
        size_t eventFieldsSize = 0;
        const auto callback = [&](uint32_t, uint32_t, Span<const Variant> eventFields) {
            notificationCount++;
            eventFieldsSize = eventFields.size();
        };

        SUBCASE("Raw") {
            const CreateMonitoredItemsRequest request(
                {},
                subId,
                TimestampsToReturn::Both,
                {
                    MonitoredItemCreateRequest(
                        {ObjectId::Server, AttributeId::EventNotifier},
                        MonitoringMode::Reporting,
                        MonitoringParameters(250, ExtensionObject::fromDecodedCopy(eventFilter))
                    ),
                }
            );
            const auto response = services::createMonitoredItemsEvent(client, request, callback);
            CHECK(response.getResponseHeader().getServiceResult().isGood());
        }
        SUBCASE("Single") {
            const auto monId =
                services::createMonitoredItemEvent(
                    client,
                    subId,
                    {ObjectId::Server, AttributeId::EventNotifier},
                    MonitoringMode::Reporting,
                    monitoringParameters,
                    callback
                )
                    .value();
            CAPTURE(monId);
        }

        Event event(server);
        event.writeTime(DateTime::now());
        event.trigger();
        client.runIterate();
        CHECK(notificationCount == 1);
        CHECK(eventFieldsSize == 3);
    }
#endif

    SUBCASE("modifyMonitoredItem") {
        const auto monId =
            services::createMonitoredItemDataChange(
                client,
                subId,
                {id, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                {}
            )
                .value();
        CAPTURE(monId);

        services::MonitoringParametersEx modifiedParameters{};
        modifiedParameters.samplingInterval = 1000.0;
        CHECK(services::modifyMonitoredItem(client, subId, monId, modifiedParameters));
    }

    SUBCASE("setMonitoringMode") {
        const auto monId =
            services::createMonitoredItemDataChange(
                client,
                subId,
                {id, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                {}
            )
                .value();
        CAPTURE(monId);
        CHECK(services::setMonitoringMode(client, subId, monId, MonitoringMode::Disabled));
    }

#if UAPP_OPEN62541_VER_GE(1, 2)
    SUBCASE("setTriggering") {
        // use current server time as triggering item and let it trigger the variable node
        size_t notificationCountTriggering = 0;
        size_t notificationCount = 0;
        const auto monIdTriggering =
            services::createMonitoredItemDataChange(
                client,
                subId,
                {VariableId::Server_ServerStatus_CurrentTime, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                [&](uint32_t, uint32_t, const DataValue&) { notificationCountTriggering++; }
            ).value();
        // set triggered item's monitoring mode to sampling
        // -> will only report if triggered by triggering item
        // https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.1.6
        const auto monId =
            services::createMonitoredItemDataChange(
                client,
                subId,
                {id, AttributeId::Value},
                MonitoringMode::Sampling,
                monitoringParameters,
                [&](uint32_t, uint32_t, const DataValue&) { notificationCount++; }
            ).value();

        client.runIterate();
        CHECK(notificationCountTriggering > 0);
        CHECK(notificationCount == 0);  // no triggering links yet

        auto result = services::setTriggering(
            client,
            subId,
            monIdTriggering,
            {monId},  // links to add
            {}  // links to remove
        );
        CHECK(result);

        client.runIterate();
#if UAPP_OPEN62541_VER_LE(1, 3)
        // TODO: fails with v1.4, why?
        CHECK(notificationCount > 0);
#endif
    }
#endif

    SUBCASE("deleteMonitoredItem") {
        CHECK(
            services::deleteMonitoredItem(client, subId, 11U).code() ==
            UA_STATUSCODE_BADMONITOREDITEMIDINVALID
        );

        bool deleted = false;
        const auto monId =
            services::createMonitoredItemDataChange(
                client,
                subId,
                {id, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                {},
                [&](uint32_t, uint32_t) { deleted = true; }
            ).value();

        CHECK(services::deleteMonitoredItem(client, subId, monId));
        client.runIterate();
        CHECK(deleted == true);
    }
}

TEST_CASE("MonitoredItem service set (server)") {
    Server server;
    const NodeId id{1, 1000};
    services::addVariable(server, {0, UA_NS0ID_OBJECTSFOLDER}, id, "Variable").value();

    services::MonitoringParametersEx monitoringParameters{};
    monitoringParameters.samplingInterval = 0.0;  // fastest

    SUBCASE("createMonitoredItemDataChange") {
        size_t notificationCount = 0;
        const auto monId =
            services::createMonitoredItemDataChange(
                server,
                0U,
                {id, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                [&](uint32_t, uint32_t, const DataValue&) { notificationCount++; }
            ).value();
        CAPTURE(monId);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        services::writeValue(server, id, Variant::fromScalar(11.11)).value();
        server.runIterate();
        CHECK(notificationCount > 0);
    }

    SUBCASE("deleteMonitoredItem") {
        CHECK(
            services::deleteMonitoredItem(server, 0U, 11U).code() ==
            UA_STATUSCODE_BADMONITOREDITEMIDINVALID
        );

        const auto monId =
            services::createMonitoredItemDataChange(
                server,
                0U,
                {id, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                {}
            )
                .value();
        CAPTURE(monId);
        CHECK(services::deleteMonitoredItem(server, 0U, monId));
    }
}
#endif
