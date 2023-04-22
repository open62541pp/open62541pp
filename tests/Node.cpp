#include <doctest/doctest.h>

#include "open62541pp/Node.h"
#include "open62541pp/Server.h"

using namespace opcua;

TEST_CASE("Node<Server>") {
    Server server;

    SUBCASE("Constructor") {
        CHECK_NOTHROW(Node<Server>(server, NodeId(0, UA_NS0ID_BOOLEAN), false));
        CHECK_NOTHROW(Node<Server>(server, NodeId(0, UA_NS0ID_BOOLEAN), true));
        CHECK_NOTHROW(Node<Server>(server, NodeId(0, "DoesNotExist"), false));
        CHECK_THROWS(Node<Server>(server, NodeId(0, "DoesNotExist"), true));
    }

    SUBCASE("Node class of default nodes") {
        CHECK(server.getRootNode().readNodeClass() == NodeClass::Object);
        CHECK(server.getObjectsNode().readNodeClass() == NodeClass::Object);
        CHECK(server.getTypesNode().readNodeClass() == NodeClass::Object);
        CHECK(server.getViewsNode().readNodeClass() == NodeClass::Object);
        CHECK(server.getObjectTypesNode().readNodeClass() == NodeClass::Object);
        CHECK(server.getVariableTypesNode().readNodeClass() == NodeClass::Object);
        CHECK(server.getDataTypesNode().readNodeClass() == NodeClass::Object);
        CHECK(server.getReferenceTypesNode().readNodeClass() == NodeClass::Object);
    }

    SUBCASE("Try read/write with node classes other than Variable") {
        CHECK_THROWS(server.getRootNode().readScalar<int>());
        CHECK_THROWS(server.getRootNode().writeScalar<int>({}));
    }

    SUBCASE("Read/write scalar") {
        auto node = server.getRootNode().addVariable({1, "testScalar"}, "testScalar");
        CHECK_NOTHROW(node.writeDataType(Type::Float));

        // Writes with wrong data type
        CHECK_THROWS(node.writeScalar<bool>({}));
        CHECK_THROWS(node.writeScalar<int>({}));

        // Writes with correct data type
        float value = 11.11f;
        CHECK_NOTHROW(node.writeScalar(value));
        CHECK(node.readScalar<float>() == value);
    }

    SUBCASE("Read/write string") {
        auto node = server.getRootNode().addVariable({1, "testString"}, "testString");
        CHECK_NOTHROW(node.writeDataType(Type::String));

        String str("test");
        CHECK_NOTHROW(node.writeScalar(str));
        CHECK(node.readScalar<std::string>() == "test");
    }

    SUBCASE("Read/write array") {
        auto node = server.getRootNode().addVariable({1, "testArray"}, "testArray");
        CHECK_NOTHROW(node.writeDataType(Type::Double));

        // Writes with wrong data type
        CHECK_THROWS(node.writeArray<int>({}));
        CHECK_THROWS(node.writeArray<float>({}));

        // Writes with correct data type
        std::vector<double> array{11.11, 22.22, 33.33};

        SUBCASE("Write as std::vector") {
            CHECK_NOTHROW(node.writeArray(array));
            CHECK(node.readArray<double>() == array);
        }

        SUBCASE("Write as raw array") {
            CHECK_NOTHROW(node.writeArray(array.data(), array.size()));
            CHECK(node.readArray<double>() == array);
        }

        SUBCASE("Write as iterator pair") {
            CHECK_NOTHROW(node.writeArray(array.begin(), array.end()));
            CHECK(node.readArray<double>() == array);
        }
    }

    SUBCASE("Equality") {
        CHECK(server.getRootNode() == server.getRootNode());
        CHECK(server.getRootNode() != server.getObjectsNode());
    }
}
