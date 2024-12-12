#include <chrono>
#include <thread>

#include <doctest/doctest.h>

#include "open62541pp/config.hpp"
#include "open62541pp/event.hpp"
#include "open62541pp/services/attribute_highlevel.hpp"  // writeValue
#include "open62541pp/services/monitoreditem.hpp"
#include "open62541pp/services/nodemanagement.hpp"  // addVariable
#include "open62541pp/services/subscription.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/ua/nodeids.hpp"

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
    REQUIRE(services::addVariable(
        server,
        {0, UA_NS0ID_OBJECTSFOLDER},
        id,
        "Variable",
        {},
        VariableTypeId::BaseDataVariableType,
        ReferenceTypeId::HasComponent
    ));

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
        CHECK(result.statusCode().isBad());
    }

    const auto subId =
        services::createSubscription(connection, subscriptionParameters, true, {}, {})
            .subscriptionId();
    CAPTURE(subId);

    SUBCASE("createMonitoredItemDataChange") {
        size_t notificationCount = 0;
        DataValue changedValue;
        const auto callback = [&](IntegerId, IntegerId, const DataValue& value) {
            notificationCount++;
            changedValue = value;
        };

        const auto createMonitoredItemDataChange = [&](auto&&... args) {
            if constexpr (isAsync<T> && UAPP_HAS_ASYNC_SUBSCRIPTIONS) {
#if UAPP_HAS_ASYNC_SUBSCRIPTIONS
                auto future = services::createMonitoredItemDataChangeAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
#endif
            } else {
                return services::createMonitoredItemDataChange(args...);
            }
        };
        const MonitoredItemCreateResult result = createMonitoredItemDataChange(
            connection,
            subId,
            ReadValueId(id, AttributeId::Value),
            MonitoringMode::Reporting,
            monitoringParameters,
            callback,
            nullptr
        );
        CHECK(result.statusCode().isGood());
        CAPTURE(result.monitoredItemId());

        services::writeValue(server, id, Variant(11.11)).throwIfBad();
        setup.client.runIterate();
        CHECK(notificationCount > 0);
        CHECK(changedValue.value().scalar<double>() == 11.11);
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
        const auto callback = [&](IntegerId, IntegerId, Span<const Variant> eventFields) {
            notificationCount++;
            eventFieldsSize = eventFields.size();
        };

        const auto createMonitoredItemEvent = [&](auto&&... args) {
            if constexpr (isAsync<T> && UAPP_HAS_ASYNC_SUBSCRIPTIONS) {
#if UAPP_HAS_ASYNC_SUBSCRIPTIONS
                auto future = services::createMonitoredItemEventAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
#endif
            } else {
                return services::createMonitoredItemEvent(args...);
            }
        };
        const MonitoredItemCreateResult result = createMonitoredItemEvent(
            connection,
            subId,
            ReadValueId(ObjectId::Server, AttributeId::EventNotifier),
            MonitoringMode::Reporting,
            monitoringParameters,
            callback,
            nullptr
        );
        CHECK(result.statusCode().isGood());
        CAPTURE(result.monitoredItemId());

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
                .monitoredItemId();
        CAPTURE(monId);

        const auto modifyMonitoredItem = [&](auto&&... args) {
            if constexpr (isAsync<T> && UAPP_HAS_ASYNC_SUBSCRIPTIONS) {
#if UAPP_HAS_ASYNC_SUBSCRIPTIONS
                auto future = services::modifyMonitoredItemAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
#endif
            } else {
                return services::modifyMonitoredItem(args...);
            }
        };

        services::MonitoringParametersEx modifiedParameters{};
        modifiedParameters.samplingInterval = 1000.0;
        const MonitoredItemModifyResult result = modifyMonitoredItem(
            connection, subId, monId, modifiedParameters
        );
        CHECK(result.statusCode().isGood());
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
                .monitoredItemId();
        CAPTURE(monId);

        const auto setMonitoringMode = [&](auto&&... args) {
            if constexpr (isAsync<T>) {
                auto future = services::setMonitoringModeAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
            } else {
                return services::setMonitoringMode(args...);
            }
        };
        const StatusCode result = setMonitoringMode(
            connection, subId, monId, MonitoringMode::Disabled
        );
        CHECK(result.isGood());
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
                [&](IntegerId, IntegerId, const DataValue&) { notificationCountTriggering++; },
                {}
            )
                .monitoredItemId();
        CAPTURE(monIdTriggering);
        // set triggered item's monitoring mode to sampling
        // -> will only report if triggered by triggering item
        // https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.1.6
        const auto monId =
            services::createMonitoredItemDataChange(
                connection,
                subId,
                {id, AttributeId::Value},
                MonitoringMode::Sampling,
                monitoringParameters,
                [&](IntegerId, IntegerId, const DataValue&) { notificationCount++; },
                {}
            )
                .monitoredItemId();
        CAPTURE(monId);

        setup.client.runIterate();
        CHECK(notificationCountTriggering > 0);
        CHECK(notificationCount == 0);  // no triggering links yet

        const auto setTriggering = [&](auto&&... args) {
            if constexpr (isAsync<T>) {
                auto future = services::setTriggeringAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
            } else {
                return services::setTriggering(args...);
            }
        };
        const SetTriggeringResponse response = setTriggering(
            connection,
            SetTriggeringRequest(
                {},
                subId,
                monIdTriggering,
                {monId},  // links to add
                {}  // links to remove
            )
        );
        CHECK(response.responseHeader().serviceResult().isGood());
        CHECK(response.addResults().size() == 1);
        CHECK(response.addResults()[0].isGood());

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
                [&](IntegerId, IntegerId) { deleted = true; }
            ).monitoredItemId();

        const auto deleteMonitoredItem = [&](auto&&... args) {
            if constexpr (isAsync<T> && UAPP_HAS_ASYNC_SUBSCRIPTIONS) {
#if UAPP_HAS_ASYNC_SUBSCRIPTIONS
                auto future = services::deleteMonitoredItemAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
#endif
            } else {
                return services::deleteMonitoredItem(args...);
            }
        };
        const StatusCode result = deleteMonitoredItem(connection, subId, monId);
        CHECK(result.isGood());
        setup.client.runIterate();
        CHECK(deleted == true);
    }
}

TEST_CASE("MonitoredItem service set (server)") {
    Server server;
    const NodeId id{1, 1000};
    REQUIRE(services::addVariable(
        server,
        {0, UA_NS0ID_OBJECTSFOLDER},
        id,
        "Variable",
        {},
        VariableTypeId::BaseDataVariableType,
        ReferenceTypeId::HasComponent
    ));

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
                [&](IntegerId, IntegerId, const DataValue&) { notificationCount++; },
                {}
            )
                .monitoredItemId();
        CAPTURE(monId);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        services::writeValue(server, id, Variant(11.11)).throwIfBad();
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
                .monitoredItemId();
        CAPTURE(monId);
        CHECK(services::deleteMonitoredItem(server, 0U, monId).isGood());
    }
}
#endif
