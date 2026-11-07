#include <array>
#include <memory>

#include <catch2/catch_test_macros.hpp>

#include "helper/server_client_setup.hpp"
#include "open62541pp/config.hpp"
#include "open62541pp/event.hpp"
#include "open62541pp/server.hpp"
#include "open62541pp/services/attribute_highlevel.hpp"
#include "open62541pp/subscription.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/ua/types.hpp"

using namespace opcua;

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

TEST_CASE("Event") {
    Server server;

    SECTION("Create and trigger event") {
        Event event{server};

        CHECK(&event.connection() == &server);
        CHECK(&std::as_const(event).connection() == &server);

        CHECK_NOTHROW(event.writeSourceName("SourceName"));
        CHECK_NOTHROW(event.writeTime(DateTime::now()));
        CHECK_NOTHROW(event.writeSeverity(200U));
        CHECK_NOTHROW(event.writeMessage({"", "Message"}));

        CHECK_NOTHROW(event.trigger());
        CHECK(event.trigger() != event.trigger());  // unique event ids
    }

#if UAPP_OPEN62541_VER_LE(1, 4)
    SECTION("Create and remove node representation") {
        auto event = std::make_unique<Event>(server);

        const auto id = event->id();
        CHECK_FALSE(id.isNull());
        CHECK(services::readNodeId(server, id));

        // delete event
        event = nullptr;
        CHECK(services::readNodeId(server, id).code() == UA_STATUSCODE_BADNODEIDUNKNOWN);
    }

    SECTION("Release ownership") {
        Event event{server};
        const auto id = event.release();
        CHECK(UA_Server_deleteNode(server.handle(), id, true) == UA_STATUSCODE_GOOD);
    }

    SECTION("Equality") {
        opcua::Event event1(server);
        opcua::Event event2(server);
        CHECK(event1 == event1);
        CHECK(event1 != event2);
    }

#endif
}

TEST_CASE("Event notification fields") {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);

    Subscription subscription{setup.client};
    const EventFilter eventFilter(
        {
            {ObjectTypeId::BaseEventType, {{0, "SourceName"}}, AttributeId::Value},
            {ObjectTypeId::BaseEventType, {{0, "Time"}}, AttributeId::Value},
            {ObjectTypeId::BaseEventType, {{0, "Severity"}}, AttributeId::Value},
            {ObjectTypeId::BaseEventType, {{0, "Message"}}, AttributeId::Value},
        },
        {}
    );

    size_t notificationCount = 0;
    std::array<Variant, 4> received;
    [[maybe_unused]]
    auto monitoredItem = subscription.subscribeEvent(
        ObjectId::Server,
        eventFilter,
        [&](IntegerId, IntegerId, const services::EventFields& eventFields) {
            notificationCount++;
#if UAPP_OPEN62541_VER_GE(1, 5)
            // event fields are addressed by their browse path
            const auto getField = [&](std::string_view name) {
                const auto* field = eventFields.get(QualifiedName(0, "/" + std::string(name)));
                return field != nullptr ? *field : Variant{};
            };
            received = {
                getField("SourceName"), getField("Time"), getField("Severity"), getField("Message")
            };
#else
            // event fields are in the order of the select clauses
            for (size_t i = 0; i < received.size() && i < eventFields.size(); ++i) {
                received[i] = eventFields[i];
            }
#endif
        }
    );

    const auto time = DateTime::now();
    Event event(setup.server, ObjectTypeId::BaseEventType);
    event.writeSourceName("test-source")
        .writeTime(time)
        .writeSeverity(777)
        .writeMessage(LocalizedText("en-US", "test message"));
    event.trigger(ObjectId::Server);

    REQUIRE(runIterateUntil(setup.client, [&] { return notificationCount > 0; }));
    CHECK(received[0].scalar<String>() == "test-source");
    CHECK(received[1].scalar<DateTime>() == time);
    CHECK(received[2].scalar<uint16_t>() == 777);
    CHECK(received[3].scalar<LocalizedText>() == LocalizedText("en-US", "test message"));
}

#if UAPP_OPEN62541_VER_GE(1, 5)
TEST_CASE("Event write property with plain name and browse path") {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);

    Subscription subscription{setup.client};
    const EventFilter eventFilter(
        {{ObjectTypeId::BaseEventType, {{0, "Severity"}}, AttributeId::Value}}, {}
    );

    size_t notificationCount = 0;
    Variant received;
    [[maybe_unused]]
    auto monitoredItem = subscription.subscribeEvent(
        ObjectId::Server,
        eventFilter,
        [&](IntegerId, IntegerId, const services::EventFields& eventFields) {
            notificationCount++;
            const auto* field = eventFields.get(QualifiedName(0, "/Severity"));
            received = field != nullptr ? *field : Variant{};
        }
    );

    // a plain name is addressed as direct child of the event ("/" is prepended),
    // a name with leading "/" is already a browse path and must not be prefixed again
    for (const auto* name : {"Severity", "/Severity"}) {
        CAPTURE(name);
        notificationCount = 0;
        received = {};

        Event event(setup.server);
        event.writeProperty({0, name}, Variant{uint16_t{321}});
        event.trigger(ObjectId::Server);

        REQUIRE(runIterateUntil(setup.client, [&] { return notificationCount > 0; }));
        REQUIRE(received.isScalar());
        CHECK(received.scalar<uint16_t>() == 321);
    }
}
#endif

#endif
