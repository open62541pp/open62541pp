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
        CHECK_NOTHROW(event.writeSourceName("SourceName"));
        CHECK_NOTHROW(event.writeTime(DateTime::now()));
        CHECK_NOTHROW(event.writeSeverity(200U));
        CHECK_NOTHROW(event.writeMessage({"", "Message"}));

        CHECK_NOTHROW(event.trigger());
        CHECK(event.trigger() != event.trigger());  // unique event ids
    }
}

#endif
