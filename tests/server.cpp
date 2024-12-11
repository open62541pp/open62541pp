#include <chrono>
#include <thread>

#include <doctest/doctest.h>

#include "open62541pp/config.hpp"
#include "open62541pp/detail/open62541/server.h"
#include "open62541pp/detail/string_utils.hpp"  // detail::toString
#include "open62541pp/node.hpp"
#include "open62541pp/plugin/accesscontrol_default.hpp"
#include "open62541pp/plugin/nodestore.hpp"
#include "open62541pp/server.hpp"
#include "open62541pp/types.hpp"

using namespace opcua;

TEST_CASE("ServerConfig") {
    SUBCASE("Construct with custom port") {
        CHECK_NOTHROW(ServerConfig(4850));
    }

    SUBCASE("Construct with custom port and certificate") {
        CHECK_NOTHROW(ServerConfig(4850, ByteString("certificate")));
    }

#ifdef UA_ENABLE_ENCRYPTION
    SUBCASE("Construct with encryption") {
        ServerConfig config(
            4850,
            {},  // certificate, invalid
            {},  // privateKey, invalid
            {},  // trustList
            {},  // issuerList
            {}  // revocationList
        );
        // no encrypting security policies enabled due to invalid certificate and key
        CHECK(config->securityPoliciesSize >= 1);
    }
#endif

    ServerConfig config;

    SUBCASE("BuildInfo") {
        config.setBuildInfo(BuildInfo(
            "productUri",
            "manufacturerName",
            "productName",
            "softwareVersion",
            "buildNumber",
            DateTime(1234)
        ));
        CHECK(detail::toString(config->buildInfo.productUri) == "productUri");
        // ...
    }

    SUBCASE("ApplicationDescription") {
        config.setApplicationUri("http://app.com");
        CHECK(detail::toString(config->applicationDescription.applicationUri) == "http://app.com");

        config.setProductUri("http://product.com");
        CHECK(detail::toString(config->applicationDescription.productUri) == "http://product.com");

        config.setApplicationName("Test App");
        CHECK(detail::toString(config->applicationDescription.applicationName.text) == "Test App");
    }

    SUBCASE("AccessControl") {
        SUBCASE("setAccessControl borrow & owning") {
            AccessControlDefault accessControl;
            config.setAccessControl(accessControl);
            config.setAccessControl(accessControl);

            config.setAccessControl(std::unique_ptr<AccessControlDefault>{});
            config.setAccessControl(std::make_unique<AccessControlDefault>());
        }

        SUBCASE("Copy user token policies to endpoints") {
            CHECK(config->endpointsSize > 0);

            // delete endpoint user identity tokens first
            for (auto& endpoint : Span(config->endpoints, config->endpointsSize)) {
                UA_Array_delete(
                    endpoint.userIdentityTokens,
                    endpoint.userIdentityTokensSize,
                    &UA_TYPES[UA_TYPES_USERTOKENPOLICY]
                );
                endpoint.userIdentityTokens = (UA_UserTokenPolicy*)UA_EMPTY_ARRAY_SENTINEL;
                endpoint.userIdentityTokensSize = 0;
            }

            AccessControlDefault accessControl;
            config.setAccessControl(accessControl);
            auto& ac = config->accessControl;

            for (const auto& endpoint : Span(config->endpoints, config->endpointsSize)) {
                CHECK(endpoint.userIdentityTokensSize == ac.userTokenPoliciesSize);
            }
        }

        SUBCASE("Use highest security policy to transfer user tokens") {
            AccessControlDefault accessControl(true, {{"user", "password"}});
            config.setAccessControl(accessControl);
            auto& ac = config->accessControl;

            CHECK(ac.userTokenPoliciesSize == 2);

            CHECK(ac.userTokenPolicies[0].tokenType == UA_USERTOKENTYPE_ANONYMOUS);
            CHECK(asWrapper<String>(ac.userTokenPolicies[0].securityPolicyUri).empty());

            CHECK(ac.userTokenPolicies[1].tokenType == UA_USERTOKENTYPE_USERNAME);
            CHECK(
                asWrapper<String>(ac.userTokenPolicies[1].securityPolicyUri) ==
                "http://opcfoundation.org/UA/SecurityPolicy#None"
            );
        }
    }
}

TEST_CASE("Server run/stop/runIterate") {
    Server server;
    CHECK_FALSE(server.isRunning());

    SUBCASE("Run/stop") {
        auto t = std::thread([&] { server.run(); });
        // wait for thread to execute run method
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CHECK(server.isRunning());
        server.stop();
        t.join();  // wait until stopped
    }

    SUBCASE("Run iterate") {
        const auto waitInterval = server.runIterate();
        CHECK(waitInterval > 0);
        CHECK(waitInterval <= 1000);
        CHECK(server.isRunning());
        server.stop();
    }

    CHECK_FALSE(server.isRunning());
}

TEST_CASE("Server methods") {
    Server server;

    SUBCASE("getNamespaceArray") {
        const auto namespaces = server.namespaceArray();
        CHECK(namespaces.size() == 2);
        CHECK(namespaces.at(0) == "http://opcfoundation.org/UA/");
        CHECK(namespaces.at(1) == "urn:open62541.server.application");
    }

    SUBCASE("registerNamespace") {
        CHECK(server.registerNamespace("test1") == 2);
        CHECK(server.namespaceArray().at(2) == "test1");

        CHECK(server.registerNamespace("test2") == 3);
        CHECK(server.namespaceArray().at(3) == "test2");
    }
}

TEST_CASE("ValueCallback") {
    Server server;

    NodeId id{1, 1000};
    auto node = Node(server, ObjectId::ObjectsFolder).addVariable(id, "testVariable");
    node.writeValueScalar<int>(1);

    bool onBeforeReadCalled = false;
    bool onAfterWriteCalled = false;
    int valueBeforeRead = 0;
    int valueAfterWrite = 0;

    ValueCallback valueCallback;
    valueCallback.onBeforeRead = [&](const DataValue& dv) {
        onBeforeReadCalled = true;
        valueBeforeRead = dv.value().scalar<int>();
    };
    valueCallback.onAfterWrite = [&](const DataValue& dv) {
        onAfterWriteCalled = true;
        valueAfterWrite = dv.value().scalar<int>();
    };
    server.setVariableNodeValueCallback(id, valueCallback);

    // trigger onBeforeRead callback with read operation
    const auto valueRead = node.readValueScalar<int>();
    CHECK(onBeforeReadCalled == true);
    CHECK(onAfterWriteCalled == false);
    CHECK(valueBeforeRead == 1);
    CHECK(valueRead == 1);

    // trigger onAfterWrite callback with write operation
    node.writeValueScalar<int>(2);
    CHECK(onBeforeReadCalled == true);
    CHECK(onAfterWriteCalled == true);
    CHECK(valueAfterWrite == 2);
}

TEST_CASE("DataSource") {
    Server server;
    NodeId id{1, 1000};
    auto node = Node(server, ObjectId::ObjectsFolder).addVariable(id, "testVariable");

    // primitive data source
    int data = 0;

    ValueBackendDataSource dataSource;
    dataSource.read = [&](DataValue& dv, const NumericRange&, bool includeSourceTimestamp) {
        dv.value().setScalar(data);
        if (includeSourceTimestamp) {
            dv.setSourceTimestamp(DateTime::now());
        }
        return UA_STATUSCODE_GOOD;
    };
    dataSource.write = [&](const DataValue& dv, const NumericRange&) {
        data = dv.value().scalar<int>();
        return UA_STATUSCODE_GOOD;
    };

    CHECK_NOTHROW(server.setVariableNodeValueBackend(id, dataSource));

    CHECK(node.readValueScalar<int>() == 0);
    CHECK_NOTHROW(node.writeValueScalar<int>(1));
    CHECK(data == 1);
}

TEST_CASE("DataSource with empty callbacks") {
    Server server;
    NodeId id{1, 1000};
    auto node = Node(server, ObjectId::ObjectsFolder).addVariable(id, "testVariable");

    server.setVariableNodeValueBackend(id, ValueBackendDataSource{});

    Variant variant;
    CHECK_THROWS_AS_MESSAGE(node.readValue(), BadStatus, "BadInternalError");
    CHECK_THROWS_AS_MESSAGE(node.writeValue(variant), BadStatus, "BadInternalError");
}

TEST_CASE("DataSource with exception in callback") {
    Server server;
    NodeId id{1, 1000};
    auto node = Node(server, ObjectId::ObjectsFolder).addVariable(id, "testVariable");

    SUBCASE("BadStatus exception") {
        ValueBackendDataSource dataSource;
        dataSource.read = [&](DataValue&, const NumericRange&, bool) -> StatusCode {
            throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
        };
        server.setVariableNodeValueBackend(id, dataSource);
        Variant variant;
        CHECK_THROWS_AS_MESSAGE(node.readValue(), BadStatus, "BadUnexpectedError");
    }

    SUBCASE("Other exception types") {
        ValueBackendDataSource dataSource;
        dataSource.read = [&](DataValue&, const NumericRange&, bool) -> StatusCode {
            throw std::runtime_error("test");
        };
        server.setVariableNodeValueBackend(id, dataSource);
        Variant variant;
        CHECK_THROWS_AS_MESSAGE(node.readValue(), BadStatus, "BadInternalError");
    }
}
