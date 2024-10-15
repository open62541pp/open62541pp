#include <chrono>
#include <string_view>
#include <thread>

#include <doctest/doctest.h>

#include "open62541pp/client.hpp"
#include "open62541pp/config.hpp"
#include "open62541pp/detail/open62541/client.h"
#include "open62541pp/plugin/accesscontrol_default.hpp"
#include "open62541pp/server.hpp"

#include "client_config.hpp"

#include "helper/server_runner.hpp"

using namespace std::chrono_literals;
using namespace opcua;

constexpr std::string_view localServerUrl{"opc.tcp://localhost:4840"};

TEST_CASE("Client discovery") {
    Server server;
    ServerRunner serverRunner(server);
    Client client;

    SUBCASE("Find servers") {
        const auto results = client.findServers(localServerUrl);
        CHECK(results.size() == 1);

        const auto& result = results[0];
        CHECK(result.getApplicationUri() == String("urn:open62541.server.application"));
        CHECK(result.getProductUri() == String("http://open62541.org"));
        CHECK(result.getApplicationType() == UA_APPLICATIONTYPE_SERVER);
        CHECK(result.getDiscoveryUrls().size() >= 1);
    }

    SUBCASE("Get endpoints") {
        const auto endpoints = client.getEndpoints(localServerUrl);
        CHECK(endpoints.size() == 1);

        const auto& endpoint = endpoints[0];
        CHECK(endpoint.getEndpointUrl() == String(localServerUrl));
        CHECK(endpoint.getServerCertificate() == ByteString());
        CHECK(endpoint.getSecurityMode() == UA_MESSAGESECURITYMODE_NONE);
        CHECK(
            endpoint.getSecurityPolicyUri() ==
            String("http://opcfoundation.org/UA/SecurityPolicy#None")
        );
        CHECK(
            endpoint.getTransportProfileUri() ==
            String("http://opcfoundation.org/UA-Profile/Transport/uatcp-uasc-uabinary")
        );
    }
}

TEST_CASE("Client connect with AnonymousIdentityToken") {
    Server server;
    ServerRunner serverRunner(server);
    Client client;

    SUBCASE("Connect with anonymous should succeed") {
        client.setUserIdentityToken(AnonymousIdentityToken{});
        CHECK_NOTHROW(client.connect(localServerUrl));
        CHECK(client.isConnected());
    }

    SUBCASE("Connect with username/password should fail") {
        client.setUserIdentityToken(UserNameIdentityToken("username", "password"));
        CHECK_THROWS(client.connect(localServerUrl));
    }
}

#if UAPP_OPEN62541_VER_GE(1, 3)
TEST_CASE("Client connect with UserNameIdentityToken") {
    AccessControlDefault accessControl(false, {{"username", "password"}});
    Server server;
    server.setAccessControl(accessControl);
    ServerRunner serverRunner(server);
    Client client;

    SUBCASE("Connect with anonymous should fail") {
        CHECK_THROWS(client.connect(localServerUrl));
    }

    SUBCASE("Connect with username/password should succeed") {
        client.setUserIdentityToken(UserNameIdentityToken("username", "password"));
        CHECK_NOTHROW(client.connect(localServerUrl));
        CHECK(client.isConnected());
    }
}
#endif

#ifdef UA_ENABLE_ENCRYPTION
TEST_CASE("Client encryption") {
    SUBCASE("Connect to unencrypted server") {
        Server server;
        ServerRunner serverRunner(server);
        Client client(ByteString{}, ByteString{}, {}, {});

        client.setSecurityMode(MessageSecurityMode::SignAndEncrypt);
        CHECK_THROWS(client.connect(localServerUrl));

        client.setSecurityMode(MessageSecurityMode::None);
        CHECK_NOTHROW(client.connect(localServerUrl));
    }

    // TODO...
}
#endif

TEST_CASE("Client run/stop") {
    Server server;
    ServerRunner serverRunner(server);
    Client client;
    client.connect(localServerUrl);

    CHECK_FALSE(client.isRunning());

    auto t = std::thread([&] { client.run(); });
    std::this_thread::sleep_for(100ms);  // wait for thread to execute run method

    CHECK(client.isRunning());

    client.stop();
    t.join();  // wait until stopped

    CHECK_FALSE(client.isRunning());
}

TEST_CASE("Client state callbacks") {
    Server server;
    ServerRunner serverRunner(server);
    Client client;

    enum class States {
        Connected,
        Disconnected,
        SessionActivated,
        SessionClosed,
    };

    std::vector<States> states;
    client.onConnected([&] { states.push_back(States::Connected); });
    client.onDisconnected([&] { states.push_back(States::Disconnected); });
    client.onSessionActivated([&] { states.push_back(States::SessionActivated); });
    client.onSessionClosed([&] { states.push_back(States::SessionClosed); });

    client.connect(localServerUrl);
    std::this_thread::sleep_for(100ms);

    // Endpoints can be discovered with FindServers (without session) before connection.
    // This will trigger the connect/disconnect before the actual connection happens.
    // -> Look at the last two states:
    CHECK(states.size() >= 2);
    CHECK(states.at(states.size() - 2) == States::Connected);
    CHECK(states.at(states.size() - 1) == States::SessionActivated);

    states.clear();
    client.disconnect();
    std::this_thread::sleep_for(100ms);

    // v1.0 will not trigger session closed
    CHECK(states.size() >= 1);
    CHECK(states.at(states.size() - 1) == States::Disconnected);
}

TEST_CASE("Client inactivity callback") {
    Server server;
    ServerRunner serverRunner(server);
    Client client;

    UA_ClientConfig* config = UA_Client_getConfig(client.handle());
    config->timeout = 50;  // ms
    config->connectivityCheckInterval = 10;  // ms

    bool inactive = false;
    client.onInactive([&] { inactive = true; });
    client.connect(localServerUrl);
    serverRunner.stop();
    client.runIterate(100);
    // TODO: v1.4 seems to ignore connectivityCheckInterval
    // check is executed after a few seconds, too slow for tests
#if UAPP_OPEN62541_VER_LE(1, 3)
    CHECK(inactive);
#endif
}

TEST_CASE("Client configuration") {
    Client client;
    UA_ClientConfig* config = UA_Client_getConfig(client.handle());
    CHECK(config != nullptr);

    SUBCASE("Set timeout") {
        client.setTimeout(333);
        CHECK(config->timeout == 333);
    }
}

TEST_CASE("Client methods") {
    Server server;
    ServerRunner serverRunner(server);
    Client client;
    client.connect(localServerUrl);

    SUBCASE("Namespace array") {
        const auto namespaces = client.getNamespaceArray();
        CHECK(namespaces.size() == 2);
        CHECK(namespaces.at(0) == "http://opcfoundation.org/UA/");
        CHECK(namespaces.at(1) == "urn:open62541.server.application");
    }
}
