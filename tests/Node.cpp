#include "catch2/catch.hpp"

#include "open62541pp/Server.h"
#include "open62541pp/Node.h"

using namespace opcua;

TEST_CASE("Node") {
    Server server;

    SECTION("Constructor") {
        REQUIRE_THROWS(Node(server, NodeId("DoesNotExist")));
    }

    SECTION("Get/set object attributes") {
        auto obj  = server.getObjectsNode();

        // add node
        NodeId testNodeId("attributesTest");
        REQUIRE_NOTHROW(obj.addObject(testNodeId, "browseName"));
        REQUIRE_NOTHROW(Node(server, testNodeId));
        auto node = Node(server, testNodeId);

        // get default attributes
        REQUIRE(node.getNodeClass()   == NodeClass::Object);
        REQUIRE(node.getBrowseName()  == "browseName");
        // REQUIRE(node.getDisplayName().empty());
        REQUIRE(node.getDescription().empty());
        REQUIRE(node.getWriteMask()   == 0);

        // set new attributes
        REQUIRE_NOTHROW(node.setBrowseName("newBrowseName"));
        REQUIRE_NOTHROW(node.setDisplayName("newDisplayName"));
        REQUIRE_NOTHROW(node.setDescription("newDescription"));
        REQUIRE_NOTHROW(node.setWriteMask(11));

        // get new attributes
        REQUIRE(node.getBrowseName()  == "newBrowseName");
        REQUIRE(node.getDisplayName() == "newDisplayName");
        REQUIRE(node.getDescription() == "newDescription");
        REQUIRE(node.getWriteMask()   == 11);
    }

    SECTION("Add/remove folder node") {
        auto node = server.getRootNode().addFolder(NodeId("testFolder"), "testFolder");
        
        REQUIRE(node.getNodeClass()   == NodeClass::Object);
        REQUIRE(node.getBrowseName()  == "testFolder");
        // REQUIRE(node.getDisplayName() == "testFolder");
        REQUIRE_NOTHROW(node.remove());
    }

    SECTION("Add/remove variable node") {
        auto node = server.getRootNode().addVariable(
            NodeId("testVariable"), "testVariable", Type::Float);

        REQUIRE(node.getNodeClass()   == NodeClass::Variable);
        REQUIRE(node.getBrowseName()  == "testVariable");
        // REQUIRE(node.getDisplayName().empty());
        REQUIRE_NOTHROW(node.remove());
    }
}