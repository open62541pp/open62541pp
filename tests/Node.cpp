#include <doctest/doctest.h>

#include "open62541pp/Client.h"
#include "open62541pp/Node.h"
#include "open62541pp/Server.h"

#include "helper/Runner.h"

using namespace opcua;

TEST_CASE("Node") {
    Server server;
    ServerRunner serverRunner(server);
    Client client;
    client.connect("opc.tcp://localhost:4840");

    // create variable node
    const NodeId varId{1, 1000};
    services::addVariable(server, {0, UA_NS0ID_OBJECTSFOLDER}, varId, "variable");
    services::writeAccessLevel(server, varId, UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE);
    services::writeWriteMask(server, varId, ~0U);  // set all bits to 1 -> allow all

    const auto testNode = [&](auto& serverOrClient) {
        SUBCASE("Constructor") {
            CHECK_NOTHROW(Node(serverOrClient, NodeId(0, UA_NS0ID_BOOLEAN), false));
            CHECK_NOTHROW(Node(serverOrClient, NodeId(0, UA_NS0ID_BOOLEAN), true));
            CHECK_NOTHROW(Node(serverOrClient, NodeId(0, "DoesNotExist"), false));
            CHECK_THROWS(Node(serverOrClient, NodeId(0, "DoesNotExist"), true));
        }

        SUBCASE("Node class of default nodes") {
            CHECK(serverOrClient.getRootNode().readNodeClass() == NodeClass::Object);
            CHECK(serverOrClient.getObjectsNode().readNodeClass() == NodeClass::Object);
            CHECK(serverOrClient.getTypesNode().readNodeClass() == NodeClass::Object);
            CHECK(serverOrClient.getViewsNode().readNodeClass() == NodeClass::Object);
            CHECK(serverOrClient.getObjectTypesNode().readNodeClass() == NodeClass::Object);
            CHECK(serverOrClient.getVariableTypesNode().readNodeClass() == NodeClass::Object);
            CHECK(serverOrClient.getDataTypesNode().readNodeClass() == NodeClass::Object);
            CHECK(serverOrClient.getReferenceTypesNode().readNodeClass() == NodeClass::Object);
        }

        SUBCASE("Get child") {
            auto root = serverOrClient.getRootNode();
            CHECK_THROWS_WITH(root.getChild({{0, "Invalid"}}), "BadNoMatch");
            CHECK_EQ(
                root.getChild({{0, "Objects"}}).getNodeId(), NodeId(0, UA_NS0ID_OBJECTSFOLDER)
            );
            CHECK_EQ(
                root.getChild({{0, "Objects"}, {0, "Server"}}).getNodeId(),
                NodeId(0, UA_NS0ID_SERVER)
            );
        }

        SUBCASE("Try read/write with node classes other than Variable") {
            auto root = serverOrClient.getRootNode();
            CHECK_THROWS(root.template readScalar<int>());
            CHECK_THROWS(root.template writeScalar<int>({}));
        }

        SUBCASE("Read/write scalar") {
            auto node = Node(serverOrClient, varId);
            CHECK_NOTHROW(node.writeDataType(Type::Float));

            // write with wrong data type
            CHECK_THROWS(node.template writeScalar<bool>({}));
            CHECK_THROWS(node.template writeScalar<int>({}));

            // write with correct data type
            float value = 11.11f;
            CHECK_NOTHROW(node.writeScalar(value));
            CHECK(node.template readScalar<float>() == value);
        }

        SUBCASE("Read/write string") {
            auto node = Node(serverOrClient, varId);
            CHECK_NOTHROW(node.writeDataType(Type::String));

            String str("test");
            CHECK_NOTHROW(node.writeScalar(str));
            CHECK(node.template readScalar<std::string>() == "test");
        }

        SUBCASE("Read/write array") {
            auto node = Node(serverOrClient, varId);
            CHECK_NOTHROW(node.writeDataType(Type::Double));

            // write with wrong data type
            CHECK_THROWS(node.template writeArray<int>({}));
            CHECK_THROWS(node.template writeArray<float>({}));

            // write with correct data type
            std::vector<double> array{11.11, 22.22, 33.33};

            SUBCASE("Write as std::vector") {
                CHECK_NOTHROW(node.writeArray(array));
                CHECK(node.template readArray<double>() == array);
            }

            SUBCASE("Write as raw array") {
                CHECK_NOTHROW(node.writeArray(array.data(), array.size()));
                CHECK(node.template readArray<double>() == array);
            }

            SUBCASE("Write as iterator pair") {
                CHECK_NOTHROW(node.writeArray(array.begin(), array.end()));
                CHECK(node.template readArray<double>() == array);
            }
        }

        SUBCASE("Equality") {
            CHECK(serverOrClient.getRootNode() == serverOrClient.getRootNode());
            CHECK(serverOrClient.getRootNode() != serverOrClient.getObjectsNode());
        }
    };

    // clang-format off
    SUBCASE("Server") { testNode(server); };
    SUBCASE("Client") { testNode(client); };
    // clang-format on
}
