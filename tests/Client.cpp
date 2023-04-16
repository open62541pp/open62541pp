#include <chrono>
#include <string_view>
#include <thread>

#include <catch2/catch_test_macros.hpp>

#include "open62541pp/Client.h"
#include "open62541pp/Server.h"

using namespace opcua;

/// Helper class to run server in background thread.
class ServerRunner {
public:
    ServerRunner(Server& server)
        : server_(server),
          thread_([&] { server_.run(); }) {
        // wait for thread to execute run method
        while (!server_.isRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    ~ServerRunner() {
        server_.stop();
        thread_.join();
    }

private:
    Server& server_;
    std::thread thread_;
};

TEST_CASE("Client") {
    Server server;
    ServerRunner serverRunner(server);
    std::string_view serverUrl{"opc.tcp://localhost:4840"};

    Client client;

    SECTION("Find servers") {
        const auto results = client.findServers(serverUrl);
        REQUIRE(results.size() == 1);

        const auto& result = results[0];
        CHECK(result.getApplicationUri() == String("urn:open62541.server.application"));
        CHECK(result.getProductUri() == String("http://open62541.org"));
        CHECK(result.getApplicationType() == UA_APPLICATIONTYPE_SERVER);
        CHECK(result.getDiscoveryUrls().size() == 1);
    }

    SECTION("Find servers should fail if connected") {
        client.connect(serverUrl);
        REQUIRE_THROWS(client.findServers(serverUrl));
    }

    SECTION("Get endpoints") {
        const auto endpoints = client.getEndpoints(serverUrl);
        REQUIRE(endpoints.size() == 1);

        const auto& endpoint = endpoints[0];
        CHECK(endpoint.getEndpointUrl() == String(serverUrl));
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

    SECTION("Get endpoints should fail if connected") {
        client.connect(serverUrl);
        REQUIRE_THROWS(client.getEndpoints(serverUrl));
    }

    SECTION("Connect/disconnect") {
        client.connect(serverUrl);
        client.disconnect();
    }

    SECTION("Connect with username/password should fail") {
        REQUIRE_THROWS(client.connect(serverUrl, {"username", "password"}));
    }
}

TEST_CASE("Client username/password") {
    Server server;
    server.setLogin({{"username", "password"}}, false);
    ServerRunner serverRunner(server);
    std::string_view serverUrl{"opc.tcp://localhost:4840"};

    Client client;
    client.connect(serverUrl, {"username", "password"});
    client.disconnect();
}
