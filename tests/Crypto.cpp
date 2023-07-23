#include <string>
#include <vector>

#include <doctest/doctest.h>

#include "open62541pp/Client.h"
#include "open62541pp/Config.h"
#include "open62541pp/Crypto.h"
#include "open62541pp/Server.h"

#include "open62541_impl.h"
#include "version.h"

#include "helper/Runner.h"

using namespace opcua;

#ifdef UAPP_CREATE_CERTIFICATE

TEST_CASE("Create certificate") {
    SUBCASE("Empty subject / subjectAltName") {
        CHECK_THROWS_WITH(crypto::createCertificate({}, {}), "BadInvalidArgument");
    }

    SUBCASE("Invalid subject / subjectAltName") {
        CHECK_THROWS_AS(
            crypto::createCertificate({String{"X=Y"}}, {String{"DNS:localhost"}}),
            CreateCertificateError
        );
        CHECK_THROWS_AS(
            crypto::createCertificate({String{"C=DE"}}, {String{"X:Y"}}),
            CreateCertificateError
        );
    }

    SUBCASE("Valid") {
        const auto cert = crypto::createCertificate(
            {
                String{"C=DE"},
                String{"O=open62541pp"},
                String{"CN=open62541ppServer@localhost"},
            },
            {
                String{"DNS:localhost"},
                String{"URI:urn:open62541.server.application"},
            },
            2048
        );
        CHECK(!cert.privateKey.empty());
        CHECK(!cert.certificate.empty());
    }
}

#endif
