#include <memory>

#include <doctest/doctest.h>

#include "open62541pp/Config.h"
#include "open62541pp/Event.h"
#include "open62541pp/Server.h"
#include "open62541pp/services/Attribute.h"

using namespace opcua;

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

TEST_CASE("Event") {
    Server server;

    SUBCASE("Create and remove node representation") {
        auto event = std::make_unique<Event>(server);

        const auto id = event->id();
        CHECK_FALSE(id.isNull());
        CHECK(services::readNodeId(server, id));

        // delete event
        event = nullptr;
        CHECK(services::readNodeId(server, id).code() == UA_STATUSCODE_BADNODEIDUNKNOWN);
    }

    SUBCASE("Create and trigger event") {
        Event event(server);

        CHECK(&event.connection() == &server);
        CHECK(&std::as_const(event).connection() == &server);

        CHECK_NOTHROW(event.writeSourceName("SourceName"));
        CHECK_NOTHROW(event.writeTime(DateTime::now()));
        CHECK_NOTHROW(event.writeSeverity(200U));
        CHECK_NOTHROW(event.writeMessage({"", "Message"}));

        CHECK_NOTHROW(event.trigger());
        CHECK(event.trigger() != event.trigger());  // unique event ids
    }

    SUBCASE("Equality") {
        opcua::Event event1(server);
        opcua::Event event2(server);
        CHECK(event1 == event1);
        CHECK(event1 != event2);
    }
}

#endif
