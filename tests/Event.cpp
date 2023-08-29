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

        const auto id = event->getNodeId();
        CHECK_FALSE(id.isNull());
        CHECK_NOTHROW(services::readNodeId(server, id));

        // delete event
        event = nullptr;
        CHECK_THROWS_WITH(services::readNodeId(server, id), "BadNodeIdUnknown");
    }

    SUBCASE("Create and trigger event") {
        Event event(server);

        CHECK(&event.getConnection() == &server);
        CHECK(&std::as_const(event).getConnection() == &server);

        CHECK_NOTHROW(event.writeSourceName("SourceName"));
        CHECK_NOTHROW(event.writeTime(DateTime::now()));
        CHECK_NOTHROW(event.writeSeverity(200U));
        CHECK_NOTHROW(event.writeMessage({"", "Message"}));

        CHECK_NOTHROW(event.trigger());
        CHECK(event.trigger() != event.trigger());  // unique event ids
    }

    SUBCASE("Equality") {
        auto event1 = server.createEvent();
        auto event2 = server.createEvent();
        CHECK(event1 == event1);
        CHECK(event1 != event2);
    }
}

#endif
