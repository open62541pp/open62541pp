#include <cstdint>
#include <string>
#include <string_view>

#include <catch2/catch_test_macros.hpp>

#include "open62541pp/client.hpp"
#include "open62541pp/config.hpp"
#include "open62541pp/plugin/create_certificate.hpp"
#include "open62541pp/server.hpp"

#include "helper/server_runner.hpp"

using namespace opcua;

#if UAPP_HAS_CREATE_CERTIFICATE

TEST_CASE("Create certificate") {
    SECTION("Empty subject / subjectAltName") {
        CHECK_THROWS_AS(createCertificate({}, {}), CreateCertificateError);
    }

    SECTION("Invalid subject / subjectAltName") {
        CHECK_THROWS_AS(
            createCertificate({String{"X=Y"}}, {String{"DNS:localhost"}}), CreateCertificateError
        );
        CHECK_THROWS_AS(
            createCertificate({String{"C=DE"}}, {String{"X:Y"}}), CreateCertificateError
        );
    }

    SECTION("Valid") {
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

    SECTION("Custom key size") {
        const auto cert = createCertificate(
            {String{"C=DE"}, String{"CN=open62541ppServer@localhost"}},
            {String{"DNS:localhost"}, String{"URI:urn:open62541.server.application"}},
            CertificateFormat::DER,
            2048
        );
        CHECK(!cert.privateKey.empty());
        CHECK(!cert.certificate.empty());
    }

    SECTION("Key size affects generated key") {
        const auto create = [](uint16_t keySizeBits, CertificateFormat format) {
            return createCertificate(
                {String{"C=DE"}, String{"CN=open62541ppServer@localhost"}},
                {String{"DNS:localhost"}, String{"URI:urn:open62541.server.application"}},
                format,
                keySizeBits
            );
        };

        const auto small = create(1024, CertificateFormat::DER);
        const auto medium = create(2048, CertificateFormat::DER);
        const auto large = create(4096, CertificateFormat::DER);
        CHECK(small.privateKey.size() < medium.privateKey.size());
        CHECK(medium.privateKey.size() < large.privateKey.size());

        // 0 -> library default (maximum), must not be smaller than an explicit size
        const auto defaultSize = create(0, CertificateFormat::DER);
        CHECK(defaultSize.privateKey.size() >= medium.privateKey.size());
    }

    SECTION("Custom key size with PEM format") {
        const auto cert = createCertificate(
            {String{"C=DE"}, String{"CN=open62541ppServer@localhost"}},
            {String{"DNS:localhost"}, String{"URI:urn:open62541.server.application"}},
            CertificateFormat::PEM,
            2048
        );
        CHECK(!cert.privateKey.empty());
        CHECK(!cert.certificate.empty());
        CHECK(
            std::string_view{cert.certificate}.find("-----BEGIN CERTIFICATE-----") !=
            std::string_view::npos
        );
    }
}

TEST_CASE("Encrypted connection server/client") {
    const std::string serverApplicationUri = "urn:localhost:server";
    const std::string clientApplicationUri = "urn:localhost:client";

    const auto setClientApplicationUri = [](ClientConfig& config, std::string_view applicationUri) {
        asWrapper<String>(config->clientDescription.applicationUri) = String{applicationUri};
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
            String{std::string{"URI:"}.append(serverApplicationUri)},
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
            String{std::string{"URI:"}.append(clientApplicationUri)},
        }
    );

    SECTION("Connect without trusting each others certificate") {
        ServerConfig serverConfig{4840, certServer.certificate, certServer.privateKey, {}, {}, {}};
        serverConfig.setApplicationUri(serverApplicationUri);
        Server server{std::move(serverConfig)};

        ClientConfig clientConfig{certClient.certificate, certClient.privateKey, {}, {}};
        clientConfig.setSecurityMode(MessageSecurityMode::SignAndEncrypt);
        setClientApplicationUri(clientConfig, clientApplicationUri);
        Client client{std::move(clientConfig)};

        {
            ServerRunner serverRunner{server};
            CHECK_THROWS(client.connect("opc.tcp://localhost:4840"));
        }
    }

    SECTION("Connect with trust lists") {
        ServerConfig serverConfig{
            4840, certServer.certificate, certServer.privateKey, {certClient.certificate}, {}, {}
        };
        serverConfig.setApplicationUri(serverApplicationUri);
        Server server{std::move(serverConfig)};

        ClientConfig clientConfig{
            certClient.certificate, certClient.privateKey, {certServer.certificate}, {}
        };
        clientConfig.setSecurityMode(MessageSecurityMode::SignAndEncrypt);
        setClientApplicationUri(clientConfig, clientApplicationUri);
        Client client{std::move(clientConfig)};

        {
            ServerRunner serverRunner{server};
            CHECK_NOTHROW(client.connect("opc.tcp://localhost:4840"));
        }
    }
}

#endif  // if UAPP_HAS_CREATE_CERTIFICATE
