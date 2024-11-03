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
TEST_CASE_TEMPLATE("MonitoredItem service set", T, Client, Async<Client>) {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& server = setup.server;
    auto& connection = setup.getInstance<T>();

    // add variable node to test data change notifications
    const NodeId id{1, 1000};
    services::addVariable(
        server,
        {0, UA_NS0ID_OBJECTSFOLDER},
        id,
        "Variable",
        {},
        VariableTypeId::BaseDataVariableType,
        ReferenceTypeId::HasComponent
    )
        .value();

    const services::SubscriptionParameters subscriptionParameters{};
    services::MonitoringParametersEx monitoringParameters{};
    monitoringParameters.samplingInterval = 0.0;  // fastest

    SUBCASE("createMonitoredItemDataChange without subscription") {
        const auto result = services::createMonitoredItemDataChange(
            connection,
            11U,  // random subId
            {id, AttributeId::Value},
            MonitoringMode::Reporting,
            monitoringParameters,
            {},
            {}
        );
        CHECK(result.getStatusCode().isBad());
    }

    const auto subId =
        services::createSubscription(connection, subscriptionParameters, true, {}, {})
            .getSubscriptionId();
    CAPTURE(subId);

    SUBCASE("createMonitoredItemDataChange") {
        size_t notificationCount = 0;
        DataValue changedValue;
        const auto callback = [&](uint32_t, uint32_t, const DataValue& value) {
            notificationCount++;
            changedValue = value;
        };

        MonitoredItemCreateResult result;
        if constexpr (isAsync<T> && UAPP_OPEN62541_VER_GE(1, 1)) {
#if UAPP_OPEN62541_VER_GE(1, 1)
            auto future = services::createMonitoredItemDataChangeAsync(
                connection,
                subId,
                {id, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                callback,
                {},
                useFuture
            );
            setup.client.runIterate();
            result = future.get();
#endif
        } else {
            result = services::createMonitoredItemDataChange(
                connection,
                subId,
                {id, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                callback,
                {}
            );
        }
        CHECK(result.getStatusCode().isGood());
        CAPTURE(result.getMonitoredItemId());

        services::writeValue(server, id, Variant::fromScalar(11.11)).throwIfBad();
        setup.client.runIterate();
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

        MonitoredItemCreateResult result;
        if constexpr (isAsync<T> && UAPP_OPEN62541_VER_GE(1, 1)) {
#if UAPP_OPEN62541_VER_GE(1, 1)
            auto future = services::createMonitoredItemEventAsync(
                connection,
                subId,
                {ObjectId::Server, AttributeId::EventNotifier},
                MonitoringMode::Reporting,
                monitoringParameters,
                callback,
                {},
                useFuture
            );
            setup.client.runIterate();
            result = future.get();
#endif
        } else {
            result = services::createMonitoredItemEvent(
                connection,
                subId,
                {ObjectId::Server, AttributeId::EventNotifier},
                MonitoringMode::Reporting,
                monitoringParameters,
                callback,
                {}
            );
        }
        CHECK(result.getStatusCode().isGood());
        CAPTURE(result.getMonitoredItemId());

        Event event(server);
        event.writeTime(DateTime::now());
        event.trigger();
        setup.client.runIterate();
        CHECK(notificationCount == 1);
        CHECK(eventFieldsSize == 3);
    }
#endif

    SUBCASE("modifyMonitoredItem") {
        const auto monId =
            services::createMonitoredItemDataChange(
                connection,
                subId,
                {id, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                {},
                {}
            )
                .getMonitoredItemId();
        CAPTURE(monId);

        services::MonitoringParametersEx modifiedParameters{};
        modifiedParameters.samplingInterval = 1000.0;
        MonitoredItemModifyResult result;
        if constexpr (isAsync<T>) {
#if UAPP_OPEN62541_VER_GE(1, 1)
            auto future = services::modifyMonitoredItemAsync(
                connection, subId, monId, modifiedParameters, useFuture
            );
            setup.client.runIterate();
            result = future.get();
#endif
        } else {
            result = services::modifyMonitoredItem(connection, subId, monId, modifiedParameters);
        }
        CHECK(result.getStatusCode().isGood());
    }

    SUBCASE("setMonitoringMode") {
        const auto monId =
            services::createMonitoredItemDataChange(
                connection,
                subId,
                {id, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                {},
                {}
            )
                .getMonitoredItemId();
        CAPTURE(monId);

        if constexpr (isAsync<T>) {
            auto future = services::setMonitoringModeAsync(
                connection, subId, monId, MonitoringMode::Disabled, useFuture
            );
            setup.client.runIterate();
            CHECK(future.get().isGood());
        } else {
            CHECK(services::setMonitoringMode(connection, subId, monId, MonitoringMode::Disabled)
                      .isGood());
        }
    }

#if UAPP_OPEN62541_VER_GE(1, 2)
    SUBCASE("setTriggering") {
        // use current server time as triggering item and let it trigger the variable node
        size_t notificationCountTriggering = 0;
        size_t notificationCount = 0;
        const auto monIdTriggering =
            services::createMonitoredItemDataChange(
                connection,
                subId,
                {VariableId::Server_ServerStatus_CurrentTime, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                [&](uint32_t, uint32_t, const DataValue&) { notificationCountTriggering++; },
                {}
            )
                .getMonitoredItemId();
        CAPTURE(monIdTriggering);
        // set triggered item's monitoring mode to sampling
        // -> will only report if triggered by triggering item
        // https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.1.6
        const auto monId =
            services::createMonitoredItemDataChange(
                connection,
                subId,
                {id, AttributeId::Value},
                MonitoringMode::Sampling,
                monitoringParameters,
                [&](uint32_t, uint32_t, const DataValue&) { notificationCount++; },
                {}
            )
                .getMonitoredItemId();
        CAPTURE(monId);

        setup.client.runIterate();
        CHECK(notificationCountTriggering > 0);
        CHECK(notificationCount == 0);  // no triggering links yet

        const SetTriggeringRequest request(
            {},
            subId,
            monIdTriggering,
            {monId},  // links to add
            {}  // links to remove
        );
        SetTriggeringResponse response;
        if constexpr (isAsync<T>) {
            auto future = services::setTriggeringAsync(connection, request, useFuture);
            setup.client.runIterate();
            response = future.get();
        } else {
            response = services::setTriggering(connection, request);
        }
        CHECK(response.getResponseHeader().getServiceResult().isGood());
        CHECK(response.getAddResults().size() == 1);
        CHECK(response.getAddResults()[0].isGood());

        setup.client.runIterate();
#if UAPP_OPEN62541_VER_LE(1, 3)
        // TODO: fails with v1.4, why?
        CHECK(notificationCount > 0);
#endif
    }
#endif

    SUBCASE("deleteMonitoredItem") {
        CHECK(
            services::deleteMonitoredItem(connection, subId, 11U) ==
            UA_STATUSCODE_BADMONITOREDITEMIDINVALID
        );

        bool deleted = false;
        const auto monId =
            services::createMonitoredItemDataChange(
                connection,
                subId,
                {id, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                {},
                [&](uint32_t, uint32_t) { deleted = true; }
            ).getMonitoredItemId();

        if constexpr (isAsync<T> && UAPP_OPEN62541_VER_GE(1, 1)) {
#if UAPP_OPEN62541_VER_GE(1, 1)
            auto future = services::deleteMonitoredItemAsync(connection, subId, monId, useFuture);
            setup.client.runIterate();
            CHECK(future.get().isGood());
#endif
        } else {
            CHECK(services::deleteMonitoredItem(connection, subId, monId).isGood());
        }
        setup.client.runIterate();
        CHECK(deleted == true);
    }
}

TEST_CASE("MonitoredItem service set (server)") {
    Server server;
    const NodeId id{1, 1000};
    services::addVariable(
        server,
        {0, UA_NS0ID_OBJECTSFOLDER},
        id,
        "Variable",
        {},
        VariableTypeId::BaseDataVariableType,
        ReferenceTypeId::HasComponent
    )
        .value();

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
                [&](uint32_t, uint32_t, const DataValue&) { notificationCount++; },
                {}
            )
                .getMonitoredItemId();
        CAPTURE(monId);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        services::writeValue(server, id, Variant::fromScalar(11.11)).throwIfBad();
        server.runIterate();
        CHECK(notificationCount > 0);
    }

    SUBCASE("deleteMonitoredItem") {
        CHECK(
            services::deleteMonitoredItem(server, 0U, 11U) ==
            UA_STATUSCODE_BADMONITOREDITEMIDINVALID
        );

        const auto monId =
            services::createMonitoredItemDataChange(
                server,
                0U,
                {id, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                {},
                {}
            )
                .getMonitoredItemId();
        CAPTURE(monId);
        CHECK(services::deleteMonitoredItem(server, 0U, monId).isGood());
    }
}
#endif
