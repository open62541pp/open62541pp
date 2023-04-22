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
            Server server(4850, "certificate...");
        }
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

    SUBCASE("Get default nodes") {
        // clang-format off
        CHECK_EQ(server.getRootNode().getNodeId(),                 NodeId{0, UA_NS0ID_ROOTFOLDER});
        CHECK_EQ(server.getObjectsNode().getNodeId(),              NodeId{0, UA_NS0ID_OBJECTSFOLDER});
        CHECK_EQ(server.getTypesNode().getNodeId(),                NodeId{0, UA_NS0ID_TYPESFOLDER});
        CHECK_EQ(server.getViewsNode().getNodeId(),                NodeId{0, UA_NS0ID_VIEWSFOLDER});
        CHECK_EQ(server.getObjectTypesNode().getNodeId(),          NodeId{0, UA_NS0ID_OBJECTTYPESFOLDER});
        CHECK_EQ(server.getVariableTypesNode().getNodeId(),        NodeId{0, UA_NS0ID_VARIABLETYPESFOLDER});
        CHECK_EQ(server.getDataTypesNode().getNodeId(),            NodeId{0, UA_NS0ID_DATATYPESFOLDER});
        CHECK_EQ(server.getReferenceTypesNode().getNodeId(),       NodeId{0, UA_NS0ID_REFERENCETYPESFOLDER});
        CHECK_EQ(server.getBaseObjectTypeNode().getNodeId(),       NodeId{0, UA_NS0ID_BASEOBJECTTYPE});
        CHECK_EQ(server.getBaseDataVariableTypeNode().getNodeId(), NodeId{0, UA_NS0ID_BASEDATAVARIABLETYPE});
        // clang-format on
    }

    SUBCASE("Equality") {
        CHECK(server == server);
        CHECK(server != Server{});
    }
}
