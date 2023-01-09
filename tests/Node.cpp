#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "open62541pp/Node.h"
#include "open62541pp/Server.h"

using namespace opcua;

TEST_CASE("Node") {
    Server server;

    SECTION("Constructor") {
        REQUIRE_NOTHROW(Node(server, NodeId(1, 0) /* boolean */));
        REQUIRE_THROWS(Node(server, NodeId("DoesNotExist", 1)));
    }

    SECTION("Get/set node attributes") {
        auto node = server.getObjectsNode().addVariable(
            {"testAttributes", 1}, "testAttributes", Type::Boolean
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
        REQUIRE(node.getValueRank() == ValueRank::Any);
        REQUIRE(node.getArrayDimensions().empty());
        REQUIRE(node.getAccessLevel() == UA_ACCESSLEVELMASK_READ);

        // set new attributes
        REQUIRE_NOTHROW(node.setDisplayName("newDisplayName"));
        REQUIRE_NOTHROW(node.setDescription("newDescription"));
        REQUIRE_NOTHROW(node.setWriteMask(11));
        REQUIRE_NOTHROW(node.setDataType(NodeId(2, 0)));
        REQUIRE_NOTHROW(node.setValueRank(ValueRank::TwoDimensions));
        REQUIRE_NOTHROW(node.setArrayDimensions({3, 2}));
        REQUIRE_NOTHROW(node.setAccessLevel(UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE));

        // get new attributes
        REQUIRE(node.getDisplayName() == "newDisplayName");
        REQUIRE(node.getDescription() == "newDescription");
        REQUIRE(node.getWriteMask() == 11);
        REQUIRE(node.getDataType() == NodeId(2, 0));
        REQUIRE(node.getValueRank() == ValueRank::TwoDimensions);
        REQUIRE(node.getArrayDimensions().size() == 2);
        REQUIRE(node.getArrayDimensions().at(0) == 3);
        REQUIRE(node.getArrayDimensions().at(1) == 2);
        REQUIRE(node.getAccessLevel() == (UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE));
    }

    SECTION("Value rank and array dimension combinations") {
        auto node = server.getObjectsNode().addVariable(
            {"testDimensions", 1}, "testDimensions", Type::Float
        );

        SECTION("Unspecified dimension (ValueRank <= 0)") {
            auto valueRank = GENERATE(
                ValueRank::Any,
                ValueRank::Scalar,
                ValueRank::ScalarOrOneDimension,
                ValueRank::OneOrMoreDimensions
            );
            CAPTURE(valueRank);
            node.setValueRank(valueRank);
            CHECK_NOTHROW(node.setArrayDimensions({}));
            CHECK_THROWS(node.setArrayDimensions({1}));
            CHECK_THROWS(node.setArrayDimensions({1, 2}));
            CHECK_THROWS(node.setArrayDimensions({1, 2, 3}));
        }

        SECTION("OneDimension") {
            node.setValueRank(ValueRank::OneDimension);
            node.setArrayDimensions({1});
            CHECK_THROWS(node.setArrayDimensions({}));
            CHECK_THROWS(node.setArrayDimensions({1, 2}));
            CHECK_THROWS(node.setArrayDimensions({1, 2, 3}));
        }

        SECTION("TwoDimensions") {
            node.setValueRank(ValueRank::TwoDimensions);
            node.setArrayDimensions({1, 2});
            CHECK_THROWS(node.setArrayDimensions({}));
            CHECK_THROWS(node.setArrayDimensions({1}));
            CHECK_THROWS(node.setArrayDimensions({1, 2, 3}));
        }

        SECTION("ThreeDimensions") {
            node.setValueRank(ValueRank::ThreeDimensions);
            node.setArrayDimensions({1, 2, 3});
            CHECK_THROWS(node.setArrayDimensions({}));
            CHECK_THROWS(node.setArrayDimensions({1}));
            CHECK_THROWS(node.setArrayDimensions({1, 2}));
        }
    }

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

    SECTION("Try read/write with node classes other than Variable") {
        REQUIRE_THROWS(server.getRootNode().read<int>());
        REQUIRE_THROWS(server.getRootNode().write<int>({}));
    }

    SECTION("Read/write scalar") {
        auto node = server.getRootNode().addVariable({"testScalar", 1}, "testScalar", Type::Float);

        // Writes with wrong data type
        REQUIRE_THROWS(node.write<bool>({}));
        REQUIRE_THROWS(node.write<int>({}));

        // Writes with correct data type
        float value = 11.11;
        REQUIRE_NOTHROW(node.write(value));
        REQUIRE(node.read<float>() == value);
    }

    SECTION("Read/write array") {
        auto node = server.getRootNode().addVariable({"testArray", 1}, "testArray", Type::Float);

        // Writes with wrong data type
        REQUIRE_THROWS(node.writeArray<int>({}));
        REQUIRE_THROWS(node.writeArray<double>({}));

        // Writes with correct data type
        std::vector<float> value{11.11, 22.22, 33.33};
        REQUIRE_NOTHROW(node.writeArray(value));
        REQUIRE(node.readArray<float>() == value);
    }

    SECTION("Remove node") {
        const NodeId id("testObj", 1);

        auto node = server.getObjectsNode().addObject(id, "obj");
        REQUIRE_NOTHROW(Node(server, id));

        node.remove();
        REQUIRE_THROWS(Node(server, id));
    }
}
