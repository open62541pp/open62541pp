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

using namespace std::chrono_literals;
using namespace opcua;

constexpr std::string_view localServerUrl{"opc.tcp://localhost:4840"};

TEST_CASE("ClientConfig constructors") {
    SUBCASE("From native") {
        ClientConfig config(UA_ClientConfig{});
    }

    SUBCASE("Default") {
        ClientConfig config;
    }

#ifdef UA_ENABLE_ENCRYPTION
    SUBCASE("Default with encryption") {
        ClientConfig config(
            {},  // certificate
            {},  // privateKey
            {},  // trustList
            {}  // revocationList
        );
    }
#endif
}

TEST_CASE("ClientConfig") {
    ClientConfig config;

    SUBCASE("Logger") {
        static size_t counter = 0;
        static LogLevel lastLogLevel{};
        static LogCategory lastLogCategory{};
        static std::string lastMessage{};

        config.setLogger([&](LogLevel level, LogCategory category, std::string_view message) {
            counter++;
            lastLogLevel = level;
            lastLogCategory = category;
            lastMessage = message;
        });

        // passing a nullptr should do nothing
        config.setLogger(nullptr);

        UA_LOG_INFO(detail::getLogger(config.handle()), UA_LOGCATEGORY_CLIENT, "Message");
        CHECK(counter == 1);
        CHECK(lastLogLevel == LogLevel::Info);
        CHECK(lastLogCategory == LogCategory::Client);
        CHECK(lastMessage == "Message");
    }

    SUBCASE("Timeout") {
        config.setTimeout(333);
        CHECK(config->timeout == 333);
    }

    SUBCASE("SecurityMode") {
        config.setSecurityMode(MessageSecurityMode::Sign);
        CHECK(config->securityMode == UA_MESSAGESECURITYMODE_SIGN);
    }

    SUBCASE("CustomDataTypes") {
        CHECK(config->customDataTypes == nullptr);

        config.setCustomDataTypes({DataType(UA_TYPES[UA_TYPES_STRING])});
        CHECK(config->customDataTypes->next == nullptr);
        CHECK(config->customDataTypes->typesSize == 1);
        CHECK(config->customDataTypes->types != nullptr);
        CHECK(config->customDataTypes->types[0].typeId == UA_TYPES[UA_TYPES_STRING].typeId);
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

    SUBCASE("Connect with anonymous should succeed") {
        ClientConfig config;
        config.setUserIdentityToken(AnonymousIdentityToken{});
        Client client(std::move(config));
        CHECK_NOTHROW(client.connect(localServerUrl));
        CHECK(client.isConnected());
    }

    SUBCASE("Connect with username/password should fail") {
        ClientConfig config;
        config.setUserIdentityToken(UserNameIdentityToken("username", "password"));
        Client client(std::move(config));
        CHECK_THROWS(client.connect(localServerUrl));
    }
}

#if UAPP_OPEN62541_VER_GE(1, 3)
TEST_CASE("Client connect with UserNameIdentityToken") {
    AccessControlDefault accessControl(false, {{"username", "password"}});
    ServerConfig serverConfig;
    serverConfig.setAccessControl(accessControl);

    Server server(std::move(serverConfig));
    ServerRunner serverRunner(server);

    SUBCASE("Connect with anonymous should fail") {
        Client client;
        CHECK_THROWS(client.connect(localServerUrl));
    }

    SUBCASE("Connect with username/password should succeed") {
        ClientConfig clientConfig;
        clientConfig.setUserIdentityToken(UserNameIdentityToken("username", "password"));
        Client client(std::move(clientConfig));
        CHECK_NOTHROW(client.connect(localServerUrl));
        CHECK(client.isConnected());
    }
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

    enum class States {
        Connected,
        Disconnected,
        SessionActivated,
        SessionClosed,
    };

    std::vector<States> states;

    ClientConfig config;
    config.onConnected([&](Client&) { states.push_back(States::Connected); });
    config.onDisconnected([&](Client&) { states.push_back(States::Disconnected); });
    config.onSessionActivated([&](Client&) { states.push_back(States::SessionActivated); });
    config.onSessionClosed([&](Client&) { states.push_back(States::SessionClosed); });

    Client client(std::move(config));
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

    ClientConfig config;
    config->timeout = 50;  // ms
    config->connectivityCheckInterval = 10;  // ms

    bool inactive = false;
    config.onInactive([&](Client&) { inactive = true; });

    Client client(std::move(config));
    client.connect(localServerUrl);
    serverRunner.stop();
    client.runIterate(100);
    // TODO: v1.4 seems to ignore connectivityCheckInterval
    // check is executed after a few seconds, too slow for tests
#if UAPP_OPEN62541_VER_LE(1, 3)
    CHECK(inactive);
#endif
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

TEST_CASE("Client helper functions") {
    UA_Client* clientNull{nullptr};
    Client client;

    CHECK(detail::getConfig(clientNull) == nullptr);
    CHECK(detail::getConfig(client.handle()) != nullptr);
    CHECK(detail::getConfig(client.handle()) == &detail::getConfig(client));

    CHECK(detail::getConnection(clientNull) == nullptr);
    CHECK(detail::getConnection(client.handle()) != nullptr);
    CHECK(detail::getConnection(client.handle()) == &detail::getConnection(client));

    CHECK(detail::getWrapper(clientNull) == nullptr);
    CHECK(detail::getWrapper(client.handle()) != nullptr);
    CHECK(detail::getWrapper(client.handle())->handle() == client.handle());

    CHECK(detail::getContext(clientNull) == nullptr);
    CHECK(detail::getContext(client.handle()) != nullptr);
    CHECK(detail::getContext(client.handle()) == &detail::getContext(client));
}
