#include <string>

#include <doctest/doctest.h>

#include "open62541pp/client.hpp"
#include "open62541pp/config.hpp"
#include "open62541pp/plugin/create_certificate.hpp"
#include "open62541pp/server.hpp"

#include "helper/server_runner.hpp"

using namespace opcua;

#if UAPP_HAS_CREATE_CERTIFICATE

TEST_CASE("Create certificate") {
    SUBCASE("Empty subject / subjectAltName") {
        CHECK_THROWS_AS(createCertificate({}, {}), CreateCertificateError);
    }

    SUBCASE("Invalid subject / subjectAltName") {
        CHECK_THROWS_AS(
            createCertificate({String{"X=Y"}}, {String{"DNS:localhost"}}), CreateCertificateError
        );
        CHECK_THROWS_AS(
            createCertificate({String{"C=DE"}}, {String{"X:Y"}}), CreateCertificateError
        );
    }

    SUBCASE("Valid") {
        const auto cert = createCertificate(
            {
                String{"C=DE"},
                String{"O=open62541pp"},
                String{"CN=open62541ppServer@localhost"},
            },
            {
                String{"DNS:localhost"},
                String{"URI:urn:open62541.server.application"},
            }
        );
        CHECK(!cert.privateKey.empty());
        CHECK(!cert.certificate.empty());
    }
}

TEST_CASE("Encrypted connection server/client") {
    const std::string serverApplicationUri = "urn:localhost:server";
    const std::string clientApplicationUri = "urn:localhost:client";

    const auto setClientApplicationUri = [](ClientConfig& config, std::string_view applicationUri) {
        asWrapper<String>(config->clientDescription.applicationUri) = String(applicationUri);
    };

    // create server certificate
    const auto certServer = createCertificate(
        {
            String{"C=DE"},
            String{"O=open62541pp"},
            String{"CN=open62541ppServer@localhost"},
        },
        {
            String{"DNS:localhost"},
            String{std::string("URI:").append(serverApplicationUri)},
        }
    );

    // create client certificate
    const auto certClient = createCertificate(
        {
            String{"C=DE"},
            String{"O=open62541pp"},
            String{"CN=open62541ppClient@localhost"},
        },
        {
            String{"DNS:localhost"},
            String{std::string("URI:").append(clientApplicationUri)},
        }
    );

    SUBCASE("Connect without trusting each others certificate") {
        ServerConfig serverConfig(4840, certServer.certificate, certServer.privateKey, {}, {}, {});
        serverConfig.setApplicationUri(serverApplicationUri);
        Server server(std::move(serverConfig));

        ClientConfig clientConfig(certClient.certificate, certClient.privateKey, {}, {});
        clientConfig.setSecurityMode(MessageSecurityMode::SignAndEncrypt);
        setClientApplicationUri(clientConfig, clientApplicationUri);
        Client client(std::move(clientConfig));

        {
            ServerRunner serverRunner(server);
            CHECK_THROWS(client.connect("opc.tcp://localhost:4840"));
        }
    }

    SUBCASE("Connect with trust lists") {
        ServerConfig serverConfig(
            4840, certServer.certificate, certServer.privateKey, {certClient.certificate}, {}, {}
        );
        serverConfig.setApplicationUri(serverApplicationUri);
        Server server(std::move(serverConfig));

        ClientConfig clientConfig(
            certClient.certificate, certClient.privateKey, {certServer.certificate}, {}
        );
        clientConfig.setSecurityMode(MessageSecurityMode::SignAndEncrypt);
        setClientApplicationUri(clientConfig, clientApplicationUri);
        Client client(std::move(clientConfig));

        {
            ServerRunner serverRunner(server);
            CHECK_NOTHROW(client.connect("opc.tcp://localhost:4840"));
        }
    }
}

#endif  // if UAPP_HAS_CREATE_CERTIFICATE
