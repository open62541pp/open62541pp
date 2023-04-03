#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "open62541pp/Node.h"
#include "open62541pp/Server.h"

using namespace opcua;

TEST_CASE("Node") {
    Server server;

    SECTION("Constructor") {
        REQUIRE_NOTHROW(Node(server, NodeId(0, UA_NS0ID_BOOLEAN)));
        REQUIRE_THROWS(Node(server, NodeId(0, "DoesNotExist")));
    }

    SECTION("Get/set node attributes") {
        auto node = server.getObjectsNode().addVariable({1, "testAttributes"}, "testAttributes");

        // get default attributes
        REQUIRE(node.readNodeClass() == NodeClass::Variable);
        REQUIRE(node.readBrowseName() == "testAttributes");
        // REQUIRE(node.readDisplayName() == LocalizedText("", "testAttributes"));
        REQUIRE(node.readDescription().getText().empty());
        REQUIRE(node.readDescription().getLocale().empty());
        REQUIRE(node.readWriteMask() == 0);
        REQUIRE(node.readDataType() == NodeId(0, UA_NS0ID_BASEDATATYPE));
        REQUIRE(node.readValueRank() == ValueRank::Any);
        REQUIRE(node.readArrayDimensions().empty());
        REQUIRE(node.readAccessLevel() == UA_ACCESSLEVELMASK_READ);

        // set new attributes
        REQUIRE_NOTHROW(node.writeDisplayName("en-US", "newDisplayName"));
        REQUIRE_NOTHROW(node.writeDescription("de-DE", "newDescription"));
        REQUIRE_NOTHROW(node.writeWriteMask(11));
        REQUIRE_NOTHROW(node.writeDataType(NodeId(0, 2)));
        REQUIRE_NOTHROW(node.writeValueRank(ValueRank::TwoDimensions));
        REQUIRE_NOTHROW(node.writeArrayDimensions({3, 2}));
        REQUIRE_NOTHROW(node.writeAccessLevel(UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE));

        // get new attributes
        REQUIRE(node.readDisplayName() == LocalizedText("en-US", "newDisplayName"));
        REQUIRE(node.readDescription() == LocalizedText("de-DE", "newDescription"));
        REQUIRE(node.readWriteMask() == 11);
        REQUIRE(node.readDataType() == NodeId(0, 2));
        REQUIRE(node.readValueRank() == ValueRank::TwoDimensions);
        REQUIRE(node.readArrayDimensions().size() == 2);
        REQUIRE(node.readArrayDimensions().at(0) == 3);
        REQUIRE(node.readArrayDimensions().at(1) == 2);
        REQUIRE(node.readAccessLevel() == (UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE));
    }

    SECTION("Value rank and array dimension combinations") {
        auto node = server.getObjectsNode().addVariable({1, "testDimensions"}, "testDimensions");

        SECTION("Unspecified dimension (ValueRank <= 0)") {
            auto valueRank = GENERATE(
                ValueRank::Any,
                ValueRank::Scalar,
                ValueRank::ScalarOrOneDimension,
                ValueRank::OneOrMoreDimensions
            );
            CAPTURE(valueRank);
            node.writeValueRank(valueRank);
            CHECK_NOTHROW(node.writeArrayDimensions({}));
            CHECK_THROWS(node.writeArrayDimensions({1}));
            CHECK_THROWS(node.writeArrayDimensions({1, 2}));
            CHECK_THROWS(node.writeArrayDimensions({1, 2, 3}));
        }

        SECTION("OneDimension") {
            node.writeValueRank(ValueRank::OneDimension);
            node.writeArrayDimensions({1});
            CHECK_THROWS(node.writeArrayDimensions({}));
            CHECK_THROWS(node.writeArrayDimensions({1, 2}));
            CHECK_THROWS(node.writeArrayDimensions({1, 2, 3}));
        }

        SECTION("TwoDimensions") {
            node.writeValueRank(ValueRank::TwoDimensions);
            node.writeArrayDimensions({1, 2});
            CHECK_THROWS(node.writeArrayDimensions({}));
            CHECK_THROWS(node.writeArrayDimensions({1}));
            CHECK_THROWS(node.writeArrayDimensions({1, 2, 3}));
        }

        SECTION("ThreeDimensions") {
            node.writeValueRank(ValueRank::ThreeDimensions);
            node.writeArrayDimensions({1, 2, 3});
            CHECK_THROWS(node.writeArrayDimensions({}));
            CHECK_THROWS(node.writeArrayDimensions({1}));
            CHECK_THROWS(node.writeArrayDimensions({1, 2}));
        }
    }

    SECTION("Node class of default nodes") {
        CHECK(server.getRootNode().readNodeClass() == NodeClass::Object);
        CHECK(server.getObjectsNode().readNodeClass() == NodeClass::Object);
        CHECK(server.getTypesNode().readNodeClass() == NodeClass::Object);
        CHECK(server.getViewsNode().readNodeClass() == NodeClass::Object);
        CHECK(server.getObjectTypesNode().readNodeClass() == NodeClass::Object);
        CHECK(server.getVariableTypesNode().readNodeClass() == NodeClass::Object);
        CHECK(server.getDataTypesNode().readNodeClass() == NodeClass::Object);
        CHECK(server.getReferenceTypesNode().readNodeClass() == NodeClass::Object);
    }

    SECTION("Get child") {
        REQUIRE_THROWS(server.getRootNode().getChild({}));
        REQUIRE_THROWS(server.getRootNode().getChild({{0, "Invalid"}}));
        REQUIRE(
            server.getRootNode().getChild({{0, "Types"}, {0, "ObjectTypes"}}) ==
            server.getObjectTypesNode()
        );
    }

    SECTION("Try read/write with node classes other than Variable") {
        REQUIRE_THROWS(server.getRootNode().readScalar<int>());
        REQUIRE_THROWS(server.getRootNode().writeScalar<int>({}));
    }

    SECTION("Read/write data value") {
        auto node = server.getRootNode().addVariable({1, "testValue"}, "testValue");

        Variant var;
        var.setScalarCopy<int>(11);
        DataValue valueWrite(var, {}, {DateTime::now()}, {}, 1, UA_STATUSCODE_GOOD);
        node.writeDataValue(valueWrite);

        DataValue valueRead;
        node.readDataValue(valueRead);

        CHECK(valueRead->hasValue);
        CHECK(valueRead->hasServerTimestamp);
        CHECK(valueRead->hasSourceTimestamp);
        CHECK(valueRead->hasServerPicoseconds);
        CHECK(valueRead->hasSourcePicoseconds);
        CHECK_FALSE(valueRead->hasStatus);  // doesn't contain error code on success

        CHECK(valueRead.getValue().value().getScalar<int>() == 11);
        CHECK(valueRead->sourceTimestamp == valueWrite->sourceTimestamp);
        CHECK(valueRead->sourcePicoseconds == valueWrite->sourcePicoseconds);
    }

    SECTION("Read/write scalar") {
        auto node = server.getRootNode().addVariable({1, "testScalar"}, "testScalar");
        node.writeDataType(Type::Float);

        // Writes with wrong data type
        REQUIRE_THROWS(node.writeScalar<bool>({}));
        REQUIRE_THROWS(node.writeScalar<int>({}));

        // Writes with correct data type
        float value = 11.11f;
        REQUIRE_NOTHROW(node.writeScalar(value));
        REQUIRE(node.readScalar<float>() == value);
    }

    SECTION("Read/write string") {
        auto node = server.getRootNode().addVariable({1, "testString"}, "testString");
        node.writeDataType(Type::String);

        String str("test");
        REQUIRE_NOTHROW(node.writeScalar(str));
        REQUIRE(node.readScalar<std::string>() == "test");
    }

    SECTION("Read/write array") {
        auto node = server.getRootNode().addVariable({1, "testArray"}, "testArray");
        node.writeDataType(Type::Double);

        // Writes with wrong data type
        REQUIRE_THROWS(node.writeArray<int>({}));
        REQUIRE_THROWS(node.writeArray<float>({}));

        // Writes with correct data type
        std::vector<double> array{11.11, 22.22, 33.33};

        SECTION("Write as std::vector") {
            REQUIRE_NOTHROW(node.writeArray(array));
            REQUIRE(node.readArray<double>() == array);
        }

        SECTION("Write as raw array") {
            REQUIRE_NOTHROW(node.writeArray(array.data(), array.size()));
            REQUIRE(node.readArray<double>() == array);
        }

        SECTION("Write as iterator pair") {
            REQUIRE_NOTHROW(node.writeArray(array.begin(), array.end()));
            REQUIRE(node.readArray<double>() == array);
        }
    }

    SECTION("Remove node") {
        const NodeId id(1, "testObj");

        auto node = server.getObjectsNode().addObject(id, "obj");
        REQUIRE_NOTHROW(Node(server, id));

        node.deleteNode();
        REQUIRE_THROWS(Node(server, id));
    }

    SECTION("Equality") {
        REQUIRE(server.getRootNode() == server.getRootNode());
        REQUIRE(server.getRootNode() != server.getObjectsNode());
    }
}
