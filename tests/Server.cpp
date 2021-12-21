#include "catch2/catch.hpp"

// turn off the -Wunused-parameter warning for open62541
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "open62541/types.h"
#include "open62541/nodeids.h"
#include "open62541/server.h"
#if __has_include("open62541/server_config.h")  // merged into server.h in v1.2
#include "open62541/server_config.h"
#endif

#pragma GCC diagnostic pop

#include "open62541pp/Server.h"
#include "open62541pp/Node.h"
#include "open62541pp/NodeId.h"
#include "open62541pp/Helper.h"

using namespace opcua;

static bool compareNodes(NodeId id, uint16_t numericId) {
    auto uaNode = UA_NODEID_NUMERIC(0, numericId);
    return UA_NodeId_equal(id.handle(), &uaNode);
}

TEST_CASE("Server") {
    Server server;

    SECTION("Start / stop server") {
        REQUIRE_FALSE(server.isRunning());
        REQUIRE_NOTHROW(server.run());
        REQUIRE_THROWS(server.run()); // already running
        REQUIRE(server.isRunning());
        REQUIRE_NOTHROW(server.stop());
    }

    SECTION("Set hostname / application name / uris") {
        auto* config = UA_Server_getConfig(server.handle());

        server.setCustomHostname("customhost");
        REQUIRE(uaStringToString(config->customHostname) == "customhost");

        server.setApplicationName("Test App");
        REQUIRE(uaStringToString(config->applicationDescription.applicationName.text) == "Test App");
        
        server.setApplicationUri("http://app.com");
        REQUIRE(uaStringToString(config->applicationDescription.applicationUri) == "http://app.com");

        server.setProductUri("http://product.com");
        REQUIRE(uaStringToString(config->applicationDescription.productUri) == "http://product.com");
    }

    SECTION("Get default nodes") {
        REQUIRE(compareNodes(server.getRootNode().getNodeId(),           UA_NS0ID_ROOTFOLDER));
        REQUIRE(compareNodes(server.getObjectsNode().getNodeId(),        UA_NS0ID_OBJECTSFOLDER));
        REQUIRE(compareNodes(server.getTypesNode().getNodeId(),          UA_NS0ID_TYPESFOLDER));
        REQUIRE(compareNodes(server.getViewsNode().getNodeId(),          UA_NS0ID_VIEWSFOLDER));
        REQUIRE(compareNodes(server.getObjectTypesNode().getNodeId(),    UA_NS0ID_OBJECTTYPESFOLDER));
        REQUIRE(compareNodes(server.getVariableTypesNode().getNodeId(),  UA_NS0ID_VARIABLETYPESFOLDER));
        REQUIRE(compareNodes(server.getDataTypesNode().getNodeId(),      UA_NS0ID_DATATYPESFOLDER));
        REQUIRE(compareNodes(server.getReferenceTypesNode().getNodeId(), UA_NS0ID_REFERENCETYPESFOLDER));
    }

    SECTION("Register namespace") {
        // namespace 0 reserved, but why starting at 2?
        REQUIRE(server.registerNamespace("testnamespaceuri1") == 2);
        REQUIRE(server.registerNamespace("testnamespaceuri2") == 3);
        REQUIRE(server.registerNamespace("testnamespaceuri3") == 4);
    }
}
