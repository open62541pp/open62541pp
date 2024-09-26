#include <doctest/doctest.h>

#include "open62541pp/client.hpp"
#include "open62541pp/config.hpp"
#include "open62541pp/server.hpp"
#include "open62541pp/session.hpp"

#include "helper/macros.hpp"  // UAPP_TSAN_ENABLED
#include "helper/server_runner.hpp"

using namespace opcua;

constexpr std::string_view localServerUrl{"opc.tcp://localhost:4840"};

TEST_CASE("Session") {
    Server server;
    ServerRunner serverRunner(server);
    Client client;

#if UAPP_OPEN62541_VER_GE(1, 3)
    SUBCASE("Get active session") {
        CHECK(server.getSessions().empty());
        client.connect(localServerUrl);
        CHECK(server.getSessions().size() == 1);
        client.disconnect();
        CHECK(server.getSessions().empty());
    }

    SUBCASE("Session attributes") {
        client.connect(localServerUrl);
        auto session = server.getSessions().at(0);

        const QualifiedName key(0, "testAttribute");
        CHECK_THROWS_WITH(session.getSessionAttribute(key), "BadNotFound");

#if UAPP_OPEN62541_VER_LE(1, 3)
        // TODO: fails with v1.4: https://github.com/open62541/open62541/issues/6724
        CHECK_NOTHROW(session.setSessionAttribute(key, Variant::fromScalar(11.11)));
        CHECK(session.getSessionAttribute(key).getScalar<double>() == 11.11);

        // retry with newly created session object
        CHECK(server.getSessions().at(0).getSessionAttribute(key).getScalar<double>() == 11.11);

        // delete session attribute
        CHECK_NOTHROW(session.deleteSessionAttribute(key));
        CHECK_THROWS_WITH(session.getSessionAttribute(key), "BadNotFound");
#endif
    }

    SUBCASE("Close session") {
        // thread sanitizer error in UA_Server_closeSession function despite mutex
        // false? bug in open62541?
#ifndef UAPP_TSAN_ENABLED
        client.connect(localServerUrl);
        auto session = server.getSessions().at(0);
        CHECK_NOTHROW(session.close());
        CHECK_THROWS_WITH(session.close(), "BadSessionIdInvalid");
#endif
    }
#endif

    SUBCASE("Equality") {
        CHECK(Session(server, {1, 1000}) == Session(server, {1, 1000}));
        CHECK(Session(server, {1, 1000}) != Session(server, {1, 1001}));
    }
}
