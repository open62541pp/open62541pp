#include <catch2/catch_test_macros.hpp>

#include "open62541pp/Node.h"
#include "open62541pp/Server.h"

using namespace opcua;

TEST_CASE("Node") {
    Server server;

    SECTION("Node class of default nodes") {
        CHECK(server.getRootNode().getNodeClass() == NodeClass::Object);
        CHECK(server.getObjectsNode().getNodeClass() == NodeClass::Object);
        CHECK(server.getTypesNode().getNodeClass() == NodeClass::Object);
        CHECK(server.getViewsNode().getNodeClass() == NodeClass::Object);
        CHECK(server.getObjectTypesNode().getNodeClass() == NodeClass::Object);
        CHECK(server.getVariableTypesNode().getNodeClass() == NodeClass::Object);
        CHECK(server.getDataTypesNode().getNodeClass() == NodeClass::Object);
        CHECK(server.getReferenceTypesNode().getNodeClass() == NodeClass::Object);
    }

    SECTION("Constructor") {
        REQUIRE_THROWS(Node(server, NodeId("DoesNotExist")));
    }

    SECTION("Add/remove node") {
        const NodeId nodeId{"testObj"};

        auto node = server.getObjectsNode().addObject(nodeId, "obj");
        REQUIRE_NOTHROW(Node(server, nodeId));

        node.remove();
        REQUIRE_THROWS(Node(server, nodeId));
    }

    SECTION("Get/set node attributes") {
        auto node = server.getObjectsNode().addVariable(
            {"testAttributes"}, "testAttributes", Type::Boolean
        );

        // get default attributes
        REQUIRE(node.getNodeClass() == NodeClass::Variable);
        REQUIRE(node.getBrowseName() == "testAttributes");
        REQUIRE(node.getDisplayName() == "testAttributes");  // default -> browse name
        REQUIRE(node.getDescription().empty());
        REQUIRE(node.getWriteMask() == 0);
        // built-in type boolean has NodeId(1, 0)
        // https://reference.opcfoundation.org/v104/Core/docs/Part6/5.1.2/
        REQUIRE(node.getDataType() == NodeId(1, 0));
        REQUIRE(node.getAccessLevel() == UA_ACCESSLEVELMASK_READ);

        // set new attributes
        REQUIRE_NOTHROW(node.setDisplayName("newDisplayName"));
        REQUIRE_NOTHROW(node.setDescription("newDescription"));
        REQUIRE_NOTHROW(node.setWriteMask(11));
        REQUIRE_NOTHROW(node.setDataType(NodeId(2, 0)));
        REQUIRE_NOTHROW(node.setAccessLevel(UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE));

        // get new attributes
        REQUIRE(node.getDisplayName() == "newDisplayName");
        REQUIRE(node.getDescription() == "newDescription");
        REQUIRE(node.getWriteMask() == 11);
        REQUIRE(node.getDataType() == NodeId(2, 0));
        REQUIRE(node.getAccessLevel() == (UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE));
    }

    SECTION("Add/remove node") {
        auto node = server.getRootNode().addObject({"testObj"}, "testObj");

        REQUIRE(node.getNodeClass() == NodeClass::Object);
        REQUIRE(node.getBrowseName() == "testObj");
        // REQUIRE(node.getDisplayName() == "testObj");
        REQUIRE_NOTHROW(node.remove());
    }

    SECTION("Try read/write with node classes other than Variable") {
        REQUIRE_THROWS(server.getRootNode().read<int>());
        REQUIRE_THROWS(server.getRootNode().write<int>({}));
    }

    SECTION("Read/write scalar") {
        auto node = server.getRootNode().addVariable(
            NodeId("testScalar"), "testScalar", Type::Float
        );

        // Writes with wrong data type
        REQUIRE_THROWS(node.write<bool>({}));
        REQUIRE_THROWS(node.write<int>({}));

        // Writes with correct data type
        float value = 11.11;
        REQUIRE_NOTHROW(node.write(value));
        REQUIRE(node.read<float>() == value);
    }

    SECTION("Read/write array") {
        auto node = server.getRootNode().addVariable(NodeId("testArray"), "testArray", Type::Float);

        // Writes with wrong data type
        REQUIRE_THROWS(node.writeArray<int>({}));
        REQUIRE_THROWS(node.writeArray<double>({}));

        // Writes with correct data type
        std::vector<float> value{11.11, 22.22, 33.33};
        REQUIRE_NOTHROW(node.writeArray(value));
        REQUIRE(node.readArray<float>() == value);
    }
}
