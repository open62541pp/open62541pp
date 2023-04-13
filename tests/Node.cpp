#include <catch2/catch_test_macros.hpp>

#include "open62541pp/Node.h"
#include "open62541pp/Server.h"

using namespace opcua;

TEST_CASE("Node<Server>") {
    Server server;

    SECTION("Constructor") {
        REQUIRE_NOTHROW(Node<Server>(server, NodeId(0, UA_NS0ID_BOOLEAN), false));
        REQUIRE_NOTHROW(Node<Server>(server, NodeId(0, UA_NS0ID_BOOLEAN), true));
        REQUIRE_NOTHROW(Node<Server>(server, NodeId(0, "DoesNotExist"), false));
        REQUIRE_THROWS(Node<Server>(server, NodeId(0, "DoesNotExist"), true));
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

    SECTION("Try read/write with node classes other than Variable") {
        REQUIRE_THROWS(server.getRootNode().readScalar<int>());
        REQUIRE_THROWS(server.getRootNode().writeScalar<int>({}));
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

    SECTION("Equality") {
        REQUIRE(server.getRootNode() == server.getRootNode());
        REQUIRE(server.getRootNode() != server.getObjectsNode());
    }
}
