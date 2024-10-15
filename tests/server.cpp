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

#include "server_config.hpp"

using namespace opcua;

TEST_CASE("ServerConfig") {
    ServerConfig config;

    SUBCASE("AccessControl") {
        SUBCASE("Set by ref or owning") {
            AccessControlDefault accessControl;
            config.setAccessControl(accessControl);
            config.setAccessControl(accessControl);

            config.setAccessControl(std::unique_ptr<AccessControlDefault>{});
            config.setAccessControl(std::make_unique<AccessControlDefault>());
        }

        SUBCASE("Copy user token policies to endpoints") {
            CHECK(config->endpointsSize > 0);

            // delete endpoint user identity tokens first
            for (size_t i = 0; i < config->endpointsSize; ++i) {
                auto& endpoint = config->endpoints[i];
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

            for (size_t i = 0; i < config->endpointsSize; ++i) {
                auto& endpoint = config->endpoints[i];
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

TEST_CASE("Server constructors") {
    SUBCASE("Construct") {
        Server server;
    }

    SUBCASE("Construct with custom port") {
        Server server(4850);
    }

    SUBCASE("Custom port and certificate") {
        Server server(4850, ByteString("certificate"));
    }
}

#ifdef UA_ENABLE_ENCRYPTION
TEST_CASE("Server encryption") {
    Server server(
        4850,
        {},  // certificate, invalid
        {},  // privateKey, invalid
        {},  // trustList
        {},  // issuerList
        {}  // revocationList
    );
    // no encrypting security policies enabled due to invalid certificate and key
    CHECK(UA_Server_getConfig(server.handle())->securityPoliciesSize >= 1);
}
#endif

TEST_CASE("Server run/stop/runIterate") {
    Server server;

    SUBCASE("Run/stop") {
        CHECK_FALSE(server.isRunning());

        auto t = std::thread([&] { server.run(); });
        std::this_thread::sleep_for(std::chrono::milliseconds(100)
        );  // wait for thread to execute run method

        CHECK(server.isRunning());

        server.stop();
        t.join();  // wait until stopped

        CHECK_FALSE(server.isRunning());
    }

    SUBCASE("Run iterate") {
        CHECK_FALSE(server.isRunning());

        for (size_t i = 0; i < 10; ++i) {
            const auto waitInterval = server.runIterate();
            CHECK(waitInterval > 0);
            CHECK(waitInterval <= 1000);
            CHECK(server.isRunning());
        }

        server.stop();
        CHECK_FALSE(server.isRunning());
    }
}

TEST_CASE("Server configuration") {
    Server server;

    SUBCASE("Set hostname / application name / uris") {
        auto* config = UA_Server_getConfig(server.handle());

        server.setApplicationName("Test App");
        CHECK(detail::toString(config->applicationDescription.applicationName.text) == "Test App");

        server.setApplicationUri("http://app.com");
        CHECK(detail::toString(config->applicationDescription.applicationUri) == "http://app.com");

        server.setProductUri("http://product.com");
        CHECK(detail::toString(config->applicationDescription.productUri) == "http://product.com");
    }

    SUBCASE("Namespace array") {
        const auto namespaces = server.getNamespaceArray();
        CHECK(namespaces.size() == 2);
        CHECK(namespaces.at(0) == "http://opcfoundation.org/UA/");
        CHECK(namespaces.at(1) == "urn:open62541.server.application");
    }

    SUBCASE("Register namespace") {
        CHECK(server.registerNamespace("test1") == 2);
        CHECK(server.getNamespaceArray().at(2) == "test1");

        CHECK(server.registerNamespace("test2") == 3);
        CHECK(server.getNamespaceArray().at(3) == "test2");
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
    valueCallback.onBeforeRead = [&](const DataValue& value) {
        onBeforeReadCalled = true;
        valueBeforeRead = value.getValue().getScalar<int>();
    };
    valueCallback.onAfterWrite = [&](const DataValue& value) {
        onAfterWriteCalled = true;
        valueAfterWrite = value.getValue().getScalar<int>();
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
    dataSource.read = [&](DataValue& value, const NumericRange&, bool includeSourceTimestamp) {
        value.getValue().setScalar(data);
        if (includeSourceTimestamp) {
            value.setSourceTimestamp(DateTime::now());
        }
        return UA_STATUSCODE_GOOD;
    };
    dataSource.write = [&](const DataValue& value, const NumericRange&) {
        data = value.getValue().getScalarCopy<int>();
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
