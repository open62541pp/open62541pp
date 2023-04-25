#include <doctest/doctest.h>

#include <algorithm>  // any_of

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
        auto rootNode = serverOrClient.getRootNode();
        auto objNode = serverOrClient.getObjectsNode();
        auto varNode = serverOrClient.getNode(varId);

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

        SUBCASE("Get references") {
            const auto refs = rootNode.getReferences();
            CHECK(refs.size() > 0);
            CHECK(std::any_of(refs.begin(), refs.end(), [&](auto& ref) {
                return ref.getBrowseName() == QualifiedName(0, "Objects");
            }));
        }

        SUBCASE("Get referenced nodes") {
            const auto nodes = objNode.getReferencedNodes();
            CHECK(nodes.size() > 0);
            CHECK(std::any_of(nodes.begin(), nodes.end(), [&](auto& node) {
                return node == rootNode;
            }));
        }

        SUBCASE("Get children") {
            CHECK(rootNode.getChildren(ReferenceType::HasChild).empty());

            const auto nodes = rootNode.getChildren(ReferenceType::HierarchicalReferences);
            CHECK(nodes.size() > 0);
            CHECK(std::any_of(nodes.begin(), nodes.end(), [&](auto& node) {
                return node == objNode;
            }));
        }

        SUBCASE("Get child") {
            CHECK_THROWS_WITH(rootNode.getChild({{0, "Invalid"}}), "BadNoMatch");
            CHECK_EQ(
                rootNode.getChild({{0, "Objects"}}).getNodeId(), NodeId(0, UA_NS0ID_OBJECTSFOLDER)
            );
            CHECK_EQ(
                rootNode.getChild({{0, "Objects"}, {0, "Server"}}).getNodeId(),
                NodeId(0, UA_NS0ID_SERVER)
            );
        }

        SUBCASE("Get parent") {
            CHECK_THROWS_WITH(rootNode.getParent(), "BadNotFound");
            CHECK_EQ(objNode.getParent(), rootNode);
        }

        SUBCASE("Try read/write with node classes other than Variable") {
            CHECK_THROWS(rootNode.template readScalar<int>());
            CHECK_THROWS(rootNode.template writeScalar<int>({}));
        }

        SUBCASE("Read/write scalar") {
            CHECK_NOTHROW(varNode.writeDataType(Type::Float));

            // write with wrong data type
            CHECK_THROWS(varNode.template writeScalar<bool>({}));
            CHECK_THROWS(varNode.template writeScalar<int>({}));

            // write with correct data type
            float value = 11.11f;
            CHECK_NOTHROW(varNode.writeScalar(value));
            CHECK(varNode.template readScalar<float>() == value);
        }

        SUBCASE("Read/write string") {
            CHECK_NOTHROW(varNode.writeDataType(Type::String));

            String str("test");
            CHECK_NOTHROW(varNode.writeScalar(str));
            CHECK(varNode.template readScalar<std::string>() == "test");
        }

        SUBCASE("Read/write array") {
            CHECK_NOTHROW(varNode.writeDataType(Type::Double));

            // write with wrong data type
            CHECK_THROWS(varNode.template writeArray<int>({}));
            CHECK_THROWS(varNode.template writeArray<float>({}));

            // write with correct data type
            std::vector<double> array{11.11, 22.22, 33.33};

            SUBCASE("Write as std::vector") {
                CHECK_NOTHROW(varNode.writeArray(array));
                CHECK(varNode.template readArray<double>() == array);
            }

            SUBCASE("Write as raw array") {
                CHECK_NOTHROW(varNode.writeArray(array.data(), array.size()));
                CHECK(varNode.template readArray<double>() == array);
            }

            SUBCASE("Write as iterator pair") {
                CHECK_NOTHROW(varNode.writeArray(array.begin(), array.end()));
                CHECK(varNode.template readArray<double>() == array);
            }
        }

        SUBCASE("Equality") {
            CHECK(rootNode == rootNode);
            CHECK(rootNode != objNode);
        }
    };

    // clang-format off
    SUBCASE("Server") { testNode(server); };
    SUBCASE("Client") { testNode(client); };
    // clang-format on
}
