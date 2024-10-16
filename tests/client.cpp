#include <chrono>
#include <string_view>
#include <thread>

#include <doctest/doctest.h>

#include "open62541pp/client.hpp"
#include "open62541pp/config.hpp"
#include "open62541pp/detail/open62541/client.h"
#include "open62541pp/plugin/accesscontrol_default.hpp"
#include "open62541pp/server.hpp"

#include "helper/server_runner.hpp"

using namespace opcua;

constexpr std::string_view localServerUrl{"opc.tcp://localhost:4840"};

TEST_CASE("ClientConfig") {
    ClientConfig config;

    SUBCASE("setTimeout") {
        config.setTimeout(333);
        CHECK(config->timeout == 333);
    }

    SUBCASE("setUserIdentityToken") {
        const auto& token = asWrapper<ExtensionObject>(config->userIdentityToken);
        CHECK(token.isEmpty());

        config.setUserIdentityToken(AnonymousIdentityToken{});
        CHECK(token.getDecodedData<AnonymousIdentityToken>() != nullptr);

        config.setUserIdentityToken(UserNameIdentityToken{});
        CHECK(token.getDecodedData<UserNameIdentityToken>() != nullptr);

        config.setUserIdentityToken(X509IdentityToken{});
        CHECK(token.getDecodedData<X509IdentityToken>() != nullptr);

        config.setUserIdentityToken(IssuedIdentityToken{});
        CHECK(token.getDecodedData<IssuedIdentityToken>() != nullptr);

        CHECK_FALSE(token.isEmpty());
    }

    SUBCASE("setSecurityMode") {
        config.setSecurityMode(MessageSecurityMode::Sign);
        CHECK(config->securityMode == UA_MESSAGESECURITYMODE_SIGN);
    }
}

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
        client.config().setUserIdentityToken(AnonymousIdentityToken{});
        CHECK_NOTHROW(client.connect(localServerUrl));
        CHECK(client.isConnected());
    }

    SUBCASE("Connect with username/password should fail") {
        client.config().setUserIdentityToken(UserNameIdentityToken("username", "password"));
        CHECK_THROWS(client.connect(localServerUrl));
    }
}

#if UAPP_OPEN62541_VER_GE(1, 3)
TEST_CASE("Client connect with UserNameIdentityToken") {
    AccessControlDefault accessControl(false, {{"username", "password"}});
    Server server;
    server.config().setAccessControl(accessControl);
    ServerRunner serverRunner(server);
    Client client;

    SUBCASE("Connect with anonymous should fail") {
        CHECK_THROWS(client.connect(localServerUrl));
    }

    SUBCASE("Connect with username/password should succeed") {
        client.config().setUserIdentityToken(UserNameIdentityToken("username", "password"));
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
        Client client(ClientConfig(ByteString{}, ByteString{}, {}, {}));

        client.config().setSecurityMode(MessageSecurityMode::SignAndEncrypt);
        CHECK_THROWS(client.connect(localServerUrl));

        client.config().setSecurityMode(MessageSecurityMode::None);
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
    // wait for thread to execute run method
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    CHECK(client.isRunning());

    client.stop();
    t.join();  // wait until stopped

    CHECK_FALSE(client.isRunning());
}

TEST_CASE("Client state callbacks") {
    Client client;

    enum class State { Connected, Disconnected, SessionActivated, SessionClosed };
    std::vector<State> states;
    client.onConnected([&] { states.push_back(State::Connected); });
    client.onDisconnected([&] { states.push_back(State::Disconnected); });
    client.onSessionActivated([&] { states.push_back(State::SessionActivated); });
    client.onSessionClosed([&] { states.push_back(State::SessionClosed); });

    SUBCASE("SecureChannel connect/disconnect") {
#if UAPP_OPEN62541_VER_LE(1, 0)
        client.config()->stateCallback(client.handle(), UA_CLIENTSTATE_CONNECTED);
        client.config()->stateCallback(client.handle(), UA_CLIENTSTATE_DISCONNECTED);
#else
        client.config()->stateCallback(client.handle(), UA_SECURECHANNELSTATE_OPEN, {}, {});
        client.config()->stateCallback(client.handle(), UA_SECURECHANNELSTATE_CLOSED, {}, {});
#endif
        CHECK(states.size() == 2);
        CHECK(states.at(0) == State::Connected);
        CHECK(states.at(1) == State::Disconnected);
    }

    SUBCASE("Session activate/close") {
#if UAPP_OPEN62541_VER_LE(1, 0)
        client.config()->stateCallback(client.handle(), UA_CLIENTSTATE_SESSION);
        client.config()->stateCallback(client.handle(), UA_CLIENTSTATE_SESSION_DISCONNECTED);
#else
        client.config()->stateCallback(client.handle(), {}, UA_SESSIONSTATE_ACTIVATED, {});
        client.config()->stateCallback(client.handle(), {}, UA_SESSIONSTATE_CLOSED, {});
#endif
        CHECK(states.size() == 2);
        CHECK(states.at(0) == State::SessionActivated);
        CHECK(states.at(1) == State::SessionClosed);
    }

    SUBCASE("Combined") {
#if UAPP_OPEN62541_VER_GE(1, 1)
        client.config()->stateCallback(
            client.handle(), UA_SECURECHANNELSTATE_OPEN, UA_SESSIONSTATE_ACTIVATED, {}
        );
        CHECK(states.size() == 2);
        CHECK(states.at(0) == State::SessionActivated);  // session state handled first
        CHECK(states.at(1) == State::Connected);
#endif
    }

    client.config()->stateCallback = nullptr;
}

TEST_CASE("Client inactivity callback") {
    Client client;
    bool inactive = false;
    client.onInactive([&] { inactive = true; });
    client.config()->inactivityCallback(client.handle());
    CHECK(inactive);
}

TEST_CASE("Client methods") {
    Server server;
    ServerRunner serverRunner(server);
    Client client;
    client.connect(localServerUrl);

    SUBCASE("getNamespaceArray") {
        const auto namespaces = client.getNamespaceArray();
        CHECK(namespaces.size() == 2);
        CHECK(namespaces.at(0) == "http://opcfoundation.org/UA/");
        CHECK(namespaces.at(1) == "urn:open62541.server.application");
    }
}
