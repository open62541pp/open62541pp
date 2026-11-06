#include <array>
#include <atomic>
#include <chrono>
#include <mutex>
#include <optional>
#include <thread>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "open62541pp/config.hpp"
#include "open62541pp/datatype.hpp"
#include "open62541pp/detail/open62541/server.h"
#include "open62541pp/detail/string_utils.hpp"  // detail::toString
#include "open62541pp/node.hpp"
#include "open62541pp/plugin/accesscontrol_default.hpp"
#include "open62541pp/plugin/nodestore.hpp"
#include "open62541pp/server.hpp"
#include "open62541pp/types.hpp"

#include "helper/server_client_setup.hpp"
#include "helper/server_runner.hpp"

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#endif

using Catch::Matchers::Message;
using namespace opcua;

TEST_CASE("ServerConfig") {
    SECTION("Construct with custom port") {
        CHECK_NOTHROW(ServerConfig{4850});
    }

    SECTION("Construct with custom port and certificate") {
        CHECK_NOTHROW(ServerConfig{4850, ByteString{"certificate"}});
    }

#ifdef UA_ENABLE_ENCRYPTION
    SECTION("Construct with encryption") {
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

    SECTION("BuildInfo") {
        config.setBuildInfo(BuildInfo{
            "productUri",
            "manufacturerName",
            "productName",
            "softwareVersion",
            "buildNumber",
            DateTime{1234}
        });
        CHECK(String{config->buildInfo.productUri} == "productUri");
        // ...
    }

    SECTION("ApplicationDescription") {
        config.setApplicationUri("http://app.com");
        CHECK(String{config->applicationDescription.applicationUri} == "http://app.com");

        config.setProductUri("http://product.com");
        CHECK(String{config->applicationDescription.productUri} == "http://product.com");

        config.setApplicationName("Test App");
        CHECK(String{config->applicationDescription.applicationName.text} == "Test App");
    }

    SECTION("AccessControl") {
        SECTION("setAccessControl borrow & owning") {
            AccessControlDefault accessControl;
            config.setAccessControl(accessControl);
            config.setAccessControl(accessControl);

            config.setAccessControl(std::unique_ptr<AccessControlDefault>{});
            config.setAccessControl(std::make_unique<AccessControlDefault>());
        }

        SECTION("Use highest security policy to transfer user tokens") {
            AccessControlDefault accessControl{true, {Login{String{"user"}, String{"password"}}}};
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
    SECTION("From native") {
        UA_ServerConfig config{};
        UA_ServerConfig_setDefault(&config);
        UA_Server* native = UA_Server_newWithConfig(&config);
        Server server{native};
    }

    SECTION("From native (nullptr)") {
        UA_Server* native{nullptr};
        CHECK_THROWS(Server{native});
    }
}

TEST_CASE("Server run/stop/runIterate") {
    Server server;
    CHECK_FALSE(server.isRunning());

    SECTION("run/stop") {
        auto t = std::thread([&] { server.run(); });
        // wait for thread to execute run method
        std::this_thread::sleep_for(std::chrono::milliseconds{100});
        CHECK(server.isRunning());
        server.stop();
        server.stop();  // should do nothing
        t.join();  // wait until stopped
    }

    // https://github.com/open62541pp/open62541pp/issues/714
    SECTION("stop before startup completes") {
        auto t = std::thread([&] { server.run(); });
        server.stop();
        t.join();
    }

    SECTION("runIterate") {
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

    SECTION("Server was run") {
        opcua::Server server;
        server.runIterate();
    }

    SECTION("Server was not run") {
        opcua::Server server;
    }

    CHECK(isWinsockActive());  // should not be affected by the server
    WSACleanup();
    CHECK(!isWinsockActive());
}
#endif

TEST_CASE("Server methods") {
    Server server;
    server.config().setApplicationUri("urn:open62541.server.application");
    ServerRunner serverRunner{server};

    SECTION("namespaceArray") {
        const auto namespaces = server.namespaceArray();
        CHECK(namespaces.size() == 2);
        CHECK(namespaces.at(0) == "http://opcfoundation.org/UA/");
        CHECK(namespaces.at(1) == "urn:open62541.server.application");
    }

    SECTION("namespaceIndex") {
        CHECK(server.namespaceIndex("http://opcfoundation.org/UA/") == 0);
        CHECK(server.namespaceIndex("urn:open62541.server.application") == 1);
        CHECK(server.namespaceIndex("nonexistent") == std::nullopt);
    }

    SECTION("registerNamespace") {
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
    auto node = Node{server, ObjectId::ObjectsFolder}.addVariable(id, "TestVariable");
    node.writeValue(Variant{1});

    auto callbackPtr = std::make_unique<ValueCallbackTest>();
    auto& callback = *callbackPtr;
    setVariableNodeValueCallback(server, id, callback);

    SECTION("move ownership") {
        setVariableNodeValueCallback(server, id, std::move(callbackPtr));
    }

    SECTION("trigger onRead callback with read operation") {
        CHECK(node.readValue().to<int>() == 1);
        CHECK(callback.onReadCalled == true);
        CHECK(callback.onWriteCalled == false);
        CHECK(callback.valueBeforeRead.value().scalar<int>() == 1);
    }

    SECTION("trigger onAfterWrite callback with write operation") {
        node.writeValue(Variant{2});
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
        dv.setValue(Variant{data});
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
    auto node = Node{server, ObjectId::ObjectsFolder}.addVariable(id, "TestVariable");

    auto sourcePtr = std::make_unique<DataSourceTest>();
    auto& source = *sourcePtr;
    setVariableNodeValueBackend(server, id, source);
    SECTION("move ownership") {
        setVariableNodeValueBackend(server, id, std::move(sourcePtr));
    }

    SECTION("read") {
        source.data = 1;
        CHECK(node.readValue().to<int>() == 1);
    }

    SECTION("write") {
        CHECK_NOTHROW(node.writeValue(Variant{2}));
        CHECK(source.data == 2);
    }

    SECTION("read/write with bad status code") {
        source.code = UA_STATUSCODE_BADINTERNALERROR;
        CHECK_THROWS_MATCHES(node.readValue(), BadStatus, Message("BadInternalError"));
        CHECK_THROWS_MATCHES(node.writeValue(Variant{2}), BadStatus, Message("BadInternalError"));
    }

    SECTION("read/write with exception in callback") {
        source.exception = BadStatus{UA_STATUSCODE_BADUNEXPECTEDERROR};
        CHECK_THROWS_MATCHES(node.readValue(), BadStatus, Message("BadUnexpectedError"));
        CHECK_THROWS_MATCHES(node.writeValue(Variant{2}), BadStatus, Message("BadUnexpectedError"));
    }
}

#if UAPP_HAS_ASYNC_OPERATIONS && UAPP_OPEN62541_VER_GE(1, 5)

struct AsyncDataSourceTest : public DataSource<true, true> {
    StatusCode read(
        [[maybe_unused]] Session& session,
        [[maybe_unused]] const NodeId& id,
        [[maybe_unused]] const NumericRange* range,
        DataValue& dv,
        bool timestamp,
        AsyncReadContext& ctx
    ) override {
        if (!deferRead) {
            dv.setValue(Variant{data.load()});
            if (timestamp) {
                dv.setSourceTimestamp(DateTime::now());
            }
            return code;
        }
        auto [op, status] = ctx.defer();
        std::thread([operation = std::move(op), value = data.load()]() mutable {
            std::this_thread::sleep_for(std::chrono::milliseconds{50});
            DataValue result;
            result.setValue(Variant{value});
            operation.complete(std::move(result));
        }).detach();
        return status;
    }

    StatusCode write(
        [[maybe_unused]] Session& session,
        [[maybe_unused]] const NodeId& id,
        [[maybe_unused]] const NumericRange* range,
        const DataValue& dv,
        AsyncWriteContext& ctx
    ) override {
        const auto newData = dv.value().scalar<int>();
        if (!deferWrite) {
            data = newData;
            return code;
        }
        auto [op, status] = ctx.defer();
        std::thread([operation = std::move(op), this, newData]() mutable {
            std::this_thread::sleep_for(std::chrono::milliseconds{50});
            data = newData;
            operation.complete();
        }).detach();
        return status;
    }

    bool deferRead = false;
    bool deferWrite = false;
    UA_StatusCode code = UA_STATUSCODE_GOOD;
    std::atomic<int> data{0};
};

TEST_CASE("DataSource async completion") {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);

    const NodeId id{1, 1000};
    [[maybe_unused]] auto localNode = Node{setup.server, ObjectId::ObjectsFolder}.addVariable(
        id,
        "TestVariable",
        VariableAttributes{}.setAccessLevel(AccessLevel::CurrentRead | AccessLevel::CurrentWrite)
    );

    AsyncDataSourceTest source;
    setVariableNodeValueBackend(setup.server, id, source);

    Node clientNode{setup.client, id};

    SECTION("Deferred read") {
        source.deferRead = true;
        source.data = 42;

        CHECK(clientNode.readValue().to<int>() == 42);
    }

    SECTION("Deferred write") {
        source.deferWrite = true;

        CHECK_NOTHROW(clientNode.writeValue(Variant{99}));
        CHECK(source.data.load() == 99);
    }
}

TEST_CASE("DataSource async read is cancelled on timeout, late completion is a no-op") {
    ServerClientSetup setup;
    setup.server.config()->asyncOperationTimeout = 300.0;  // ms, short to avoid hanging
    setup.client.connect(setup.endpointUrl);

    const NodeId id{1, 1000};
    [[maybe_unused]] auto localNode = Node{setup.server, ObjectId::ObjectsFolder}.addVariable(
        id, "TestVariable"
    );

    struct Captured {
        std::mutex mutex;
        std::optional<AsyncReadOperation> operation;
    };

    auto captured = std::make_shared<Captured>();

    struct NeverCompletesDataSource : public DataSource<true, false> {
        explicit NeverCompletesDataSource(std::shared_ptr<Captured> c)
            : captured{std::move(c)} {}

        StatusCode read(
            Session&, const NodeId&, const NumericRange*, DataValue&, bool, AsyncReadContext& ctx
        ) override {
            auto [operation, status] = ctx.defer();
            std::lock_guard lock{captured->mutex};
            captured->operation = std::move(operation);  // intentionally never completed
            return status;
        }

        StatusCode write(Session&, const NodeId&, const NumericRange*, const DataValue&) override {
            return UA_STATUSCODE_GOOD;
        }

        std::shared_ptr<Captured> captured;
    };

    NeverCompletesDataSource source{captured};
    setVariableNodeValueBackend(setup.server, id, source);

    // blocks until the server responds, which is not before the operation is cancelled
    // (the returned status is irrelevant for this test)
    try {
        Node{setup.client, id}.readValue();
    } catch (...) {
    }

    std::lock_guard lock{captured->mutex};
    REQUIRE(captured->operation.has_value());
    CHECK(captured->operation->cancelled());
    CHECK_NOTHROW(captured->operation->complete(DataValue{}));
}

#endif  // UAPP_HAS_ASYNC_OPERATIONS && UAPP_OPEN62541_VER_GE(1, 5)

TEST_CASE("Server teardown with custom struct type and a stored variable node") {
    struct Point {
        std::int32_t x;
        std::int32_t y;
    };

    Server server;

    auto pointType =
        DataTypeBuilder<Point>::createStructure("_Point", {1, 3001}, {1, 3002})
            .addField<&Point::x>("x")
            .addField<&Point::y>("y")
            .build();
    server.config().addCustomDataTypes({pointType});

    const NodeId pointId{1, 3001};
    const UA_DataType* pointRegistered = findDataType(server, pointId);
    REQUIRE(pointRegistered != nullptr);

    Point value{1, 2};

    CHECK_NOTHROW(Node{server, ObjectId::ObjectsFolder}.addVariable(
        {1, 5001},
        "point",
        VariableAttributes{}
            .setDataType(pointId)
            .setValueRank(ValueRank::Scalar)
            .setValue(Variant{value, *pointRegistered})
    ));
}
