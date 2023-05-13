#include <string_view>

#include <doctest/doctest.h>

#include "open62541pp/Client.h"
#include "open62541pp/Server.h"

#include "helper/Runner.h"

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
        CHECK(result.getDiscoveryUrls().size() == 1);
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

TEST_CASE("Client anonymous login") {
    Server server;
    ServerRunner serverRunner(server);
    Client client;

    SUBCASE("Connect/disconnect") {
        CHECK_FALSE(client.isConnected());
        CHECK_NOTHROW(client.connect(localServerUrl));
        CHECK(client.isConnected());
        CHECK_NOTHROW(client.disconnect());
    }

    SUBCASE("Connect with username/password should fail") {
        CHECK_THROWS(client.connect(localServerUrl, {"username", "password"}));
    }
}

TEST_CASE("Client username/password login") {
    Server server;
    server.setLogin({{"username", "password"}}, false);
    ServerRunner serverRunner(server);
    Client client;

    SUBCASE("Connect with anonymous should fail") {
        CHECK_THROWS(client.connect(localServerUrl));
    }

    SUBCASE("Connect with username/password should succeed") {
        CHECK_NOTHROW(client.connect(localServerUrl, {"username", "password"}));
        CHECK(client.isConnected());
        CHECK_NOTHROW(client.disconnect());
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
