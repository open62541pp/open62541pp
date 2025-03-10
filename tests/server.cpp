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

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#endif

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

    SUBCASE("run/stop") {
        auto t = std::thread([&] { server.run(); });
        // wait for thread to execute run method
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CHECK(server.isRunning());
        server.stop();
        server.stop();  // should do nothing
        t.join();  // wait until stopped
    }

    SUBCASE("runIterate") {
        const auto waitInterval = server.runIterate();
        CHECK(waitInterval > 0);
        CHECK(waitInterval <= 1000);
        CHECK(server.isRunning());
        server.stop();
        server.stop();  // should do nothing
    }

    CHECK_FALSE(server.isRunning());
}

#ifdef _WIN32
static bool isWinsockActive() {
    SOCKET testSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (testSocket == INVALID_SOCKET) {
        int error = WSAGetLastError();
        if (error == WSANOTINITIALISED) {
            return false;
        }
    } else {
        closesocket(testSocket);  // Close the test socket if it was created successfully
    }
    return true;
}

TEST_CASE("Server with Winsock (Windows only)") {
    // https://github.com/open62541pp/open62541pp/issues/547
    WSADATA wsaData;
    CHECK(WSAStartup(MAKEWORD(2, 2), &wsaData) == NO_ERROR);
    CHECK(isWinsockActive());

    SUBCASE("Server was run") {
        opcua::Server server;
        server.runIterate();
    }

    SUBCASE("Server was not run") {
        opcua::Server server;
    }

    CHECK(isWinsockActive());  // should not be affected by the server
    WSACleanup();
    CHECK(!isWinsockActive());
}
#endif

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

struct ValueCallbackTest : public ValueCallbackBase {
    void onRead(
        [[maybe_unused]] Session& session,
        [[maybe_unused]] const NodeId& id,
        [[maybe_unused]] const NumericRange* range,
        const DataValue& value
    ) override {
        onReadCalled = true;
        valueBeforeRead = value;
    }

    void onWrite(
        [[maybe_unused]] Session& session,
        [[maybe_unused]] const NodeId& id,
        [[maybe_unused]] const NumericRange* range,
        const DataValue& value
    ) override {
        onWriteCalled = true;
        valueAfterWrite = value;
    }

    bool onReadCalled = false;
    bool onWriteCalled = false;

    DataValue valueBeforeRead;
    DataValue valueAfterWrite;
};

TEST_CASE("ValueCallback") {
    Server server;

    const NodeId id{1, 1000};
    auto node = Node(server, ObjectId::ObjectsFolder).addVariable(id, "TestVariable");
    node.writeValueScalar<int>(1);

    auto callbackPtr = std::make_unique<ValueCallbackTest>();
    auto& callback = *callbackPtr;
    server.setVariableNodeValueCallback(id, callback);

    SUBCASE("move ownership") {
        server.setVariableNodeValueCallback(id, std::move(callbackPtr));
    }

    SUBCASE("trigger onRead callback with read operation") {
        CHECK(node.readValueScalar<int>() == 1);
        CHECK(callback.onReadCalled == true);
        CHECK(callback.onWriteCalled == false);
        CHECK(callback.valueBeforeRead.value().scalar<int>() == 1);
    }

    SUBCASE("trigger onAfterWrite callback with write operation") {
        node.writeValueScalar<int>(2);
        CHECK(callback.onReadCalled == false);
        CHECK(callback.onWriteCalled == true);
        CHECK(callback.valueAfterWrite.value().scalar<int>() == 2);
    }
}

struct DataSourceTest : public DataSourceBase {
    StatusCode read(
        [[maybe_unused]] Session& session,
        [[maybe_unused]] const NodeId& id,
        [[maybe_unused]] const NumericRange* range,
        DataValue& dv,
        bool timestamp
    ) override {
        dv.setValue(Variant(data));
        if (timestamp) {
            dv.setSourceTimestamp(DateTime::now());
        }
        if (exception.has_value()) {
            throw exception.value();
        }
        return code;
    }

    StatusCode write(
        [[maybe_unused]] Session& session,
        [[maybe_unused]] const NodeId& id,
        [[maybe_unused]] const NumericRange* range,
        const DataValue& dv
    ) override {
        data = dv.value().scalar<int>();
        if (exception.has_value()) {
            throw exception.value();
        }
        return code;
    }

    std::optional<BadStatus> exception;
    UA_StatusCode code = UA_STATUSCODE_GOOD;
    int data = 0;
};

TEST_CASE("DataSource") {
    Server server;

    const NodeId id{1, 1000};
    auto node = Node(server, ObjectId::ObjectsFolder).addVariable(id, "TestVariable");

    auto sourcePtr = std::make_unique<DataSourceTest>();
    auto& source = *sourcePtr;
    server.setVariableNodeDataSource(id, source);
    SUBCASE("move ownership") {
        server.setVariableNodeDataSource(id, std::move(sourcePtr));
    }

    SUBCASE("read") {
        source.data = 1;
        CHECK(node.readValueScalar<int>() == 1);
    }

    SUBCASE("write") {
        CHECK_NOTHROW(node.writeValueScalar<int>(2));
        CHECK(source.data == 2);
    }

    SUBCASE("read/write with bad status code") {
        source.code = UA_STATUSCODE_BADINTERNALERROR;
        CHECK_THROWS_WITH_AS(node.readValueScalar<int>(), "BadInternalError", BadStatus);
        CHECK_THROWS_WITH_AS(node.writeValueScalar<int>(2), "BadInternalError", BadStatus);
    }

    SUBCASE("read/write with exception in callback") {
        source.exception = BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
        CHECK_THROWS_WITH_AS(node.readValueScalar<int>(), "BadUnexpectedError", BadStatus);
        CHECK_THROWS_WITH_AS(node.writeValueScalar<int>(2), "BadUnexpectedError", BadStatus);
    }
}
