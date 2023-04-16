#include <atomic>
#include <string_view>
#include <thread>

#include <catch2/catch_test_macros.hpp>

#include "open62541pp/Client.h"
#include "open62541pp/Server.h"

using namespace opcua;

/// Helper class to run server in background thread.
class ServerRunner {
public:
    ServerRunner(Server& server) {
        server.setLogger({});  // disable logging to prevent data races
        server.runIterate();  // make sure server is running within constructor
        thread_ = std::thread([&] {
            while (!stopFlag_) {
                server.runIterate();
                // no sleep here, process server events as fast a possible
            };
        });
    }

    ~ServerRunner() {
        stopFlag_ = true;
        thread_.join();
    }

private:
    std::atomic<bool> stopFlag_{false};
    std::thread thread_;
};

constexpr std::string_view localServerUrl{"opc.tcp://localhost:4840"};

TEST_CASE("Client discovery") {
    Server server;
    ServerRunner serverRunner(server);

    Client client;

    SECTION("Find servers") {
        const auto results = client.findServers(localServerUrl);
        REQUIRE(results.size() == 1);

        const auto& result = results[0];
        CHECK(result.getApplicationUri() == String("urn:open62541.server.application"));
        CHECK(result.getProductUri() == String("http://open62541.org"));
        CHECK(result.getApplicationType() == UA_APPLICATIONTYPE_SERVER);
        CHECK(result.getDiscoveryUrls().size() == 1);
    }

    SECTION("Get endpoints") {
        const auto endpoints = client.getEndpoints(localServerUrl);
        REQUIRE(endpoints.size() == 1);

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

TEST_CASE("Client anonymous login") {
    Server server;
    ServerRunner serverRunner(server);

    Client client;

    SECTION("Connect/disconnect") {
        client.connect(localServerUrl);
        client.disconnect();
    }

    SECTION("Connect with username/password should fail") {
        REQUIRE_THROWS(client.connect(localServerUrl, {"username", "password"}));
    }
}

TEST_CASE("Client username/password login") {
    Server server;
    server.setLogin({{"username", "password"}}, false);
    ServerRunner serverRunner(server);

    Client client;

    SECTION("Connect with anonymous should fail") {
        REQUIRE_THROWS(client.connect(localServerUrl));
    }

    SECTION("Connect with username/password should succeed") {
        client.connect(localServerUrl, {"username", "password"});
        client.disconnect();
    }
}
