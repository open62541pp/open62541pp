#include <chrono>
#include <thread>

#include <doctest/doctest.h>

#include "open62541pp/Node.h"
#include "open62541pp/Server.h"
#include "open62541pp/detail/helper.h"
#include "open62541pp/types/NodeId.h"

#include "open62541_impl.h"

using namespace std::chrono_literals;
using namespace opcua;

TEST_CASE("Server") {
    SUBCASE("Constructors") {
        SUBCASE("Default") {
            Server server;
        }
        SUBCASE("Custom port") {
            Server server(4850);
        }
        SUBCASE("Custom port and certificate") {
            Server server(4850, ByteString("certificate"));
        }
#ifdef UA_ENABLE_ENCRYPTION
        SUBCASE("With encryption (invalid)") {
            Server server(
                4850,
                ByteString("certificate"),  // invalid
                ByteString("privateKey"),  // invalid
                {},
                {},
                {}
            );
            // no encrypting security policies enabled due to invalid certificate and key
            CHECK(UA_Server_getConfig(server.handle())->securityPoliciesSize == 1);
        }
#endif
    }

    Server server;

    SUBCASE("Run/stop") {
        CHECK_FALSE(server.isRunning());

        auto t = std::thread([&] { server.run(); });
        std::this_thread::sleep_for(100ms);  // wait for thread to execute run method

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
            CHECK(waitInterval <= 50);
            CHECK(server.isRunning());
        }

        server.stop();
        CHECK_FALSE(server.isRunning());
    }

    SUBCASE("Set hostname / application name / uris") {
        auto* config = UA_Server_getConfig(server.handle());

        server.setCustomHostname("customhost");
        CHECK(detail::toString(config->customHostname) == "customhost");

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

    SUBCASE("Variable node value callback") {
        NodeId id{1, 1000};
        auto node = server.getObjectsNode().addVariable(id, "testVariable");
        node.writeScalar<int>(1);

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
        const auto valueRead = node.readScalar<int>();
        CHECK(onBeforeReadCalled == true);
        CHECK(onAfterWriteCalled == false);
        CHECK(valueBeforeRead == 1);
        CHECK(valueRead == 1);

        // trigger onAfterWrite callback with write operation
        node.writeScalar<int>(2);
        CHECK(onBeforeReadCalled == true);
        CHECK(onAfterWriteCalled == true);
        CHECK(valueAfterWrite == 2);
    }

    SUBCASE("Get default nodes") {
        // clang-format off
        CHECK_EQ(server.getRootNode().getNodeId(),    NodeId{0, UA_NS0ID_ROOTFOLDER});
        CHECK_EQ(server.getObjectsNode().getNodeId(), NodeId{0, UA_NS0ID_OBJECTSFOLDER});
        CHECK_EQ(server.getTypesNode().getNodeId(),   NodeId{0, UA_NS0ID_TYPESFOLDER});
        CHECK_EQ(server.getViewsNode().getNodeId(),   NodeId{0, UA_NS0ID_VIEWSFOLDER});
        // clang-format on
    }

    SUBCASE("Equality") {
        CHECK(server == server);
        CHECK(server != Server{});
    }
}
