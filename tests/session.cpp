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

    SUBCASE("Construct") {
        const NodeId sessionId{1, 1000};
        int sessionContext = 11;
        Session session(server, sessionId, &sessionContext);

        CHECK(session.connection() == server);
        CHECK(std::as_const(session).connection() == server);

        CHECK(session.id() == sessionId);

        CHECK(session.context() == &sessionContext);
        CHECK(std::as_const(session).context() == &sessionContext);
    }

#if UAPP_OPEN62541_VER_GE(1, 3)
    SUBCASE("Get active session") {
        CHECK(server.sessions().empty());
        client.connect(localServerUrl);
        CHECK(server.sessions().size() == 1);
        client.disconnect();
        CHECK(server.sessions().empty());
    }

    SUBCASE("Session attributes") {
        client.connect(localServerUrl);
        auto session = server.sessions().at(0);

        const QualifiedName key(0, "testAttribute");
        CHECK_THROWS_WITH(session.getSessionAttribute(key), "BadNotFound");

#if UAPP_OPEN62541_VER_LE(1, 3)
        // TODO: fails with v1.4: https://github.com/open62541/open62541/issues/6724
        CHECK_NOTHROW(session.setSessionAttribute(key, Variant(11.11)));
        CHECK(session.getSessionAttribute(key).scalar<double>() == 11.11);

        // retry with newly created session object
        CHECK(server.sessions().at(0).getSessionAttribute(key).scalar<double>() == 11.11);

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
        auto session = server.sessions().at(0);
        CHECK_NOTHROW(session.close());
        CHECK_THROWS_WITH(session.close(), "BadSessionIdInvalid");
#endif
    }
#endif

    SUBCASE("Equality") {
        CHECK(Session(server, {1, 1000}, nullptr) == Session(server, {1, 1000}, nullptr));
        CHECK(Session(server, {1, 1000}, nullptr) != Session(server, {1, 1001}, nullptr));
    }
}
