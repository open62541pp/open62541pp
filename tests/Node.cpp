#include <doctest/doctest.h>

#include <algorithm>  // any_of

#include "open62541pp/Client.h"
#include "open62541pp/Config.h"
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
    const NodeId varId{1, 1};
    services::addVariable(server, {0, UA_NS0ID_OBJECTSFOLDER}, varId, "variable");
    services::writeAccessLevel(server, varId, 0xFF);
    services::writeWriteMask(server, varId, 0xFFFFFFFF);  // set all bits to 1 -> allow all

    const auto testNode = [&](auto& serverOrClient) {
        auto rootNode = serverOrClient.getRootNode();
        auto objNode = serverOrClient.getObjectsNode();
        auto varNode = serverOrClient.getNode(varId);
        auto refNode = serverOrClient.getNode(ReferenceTypeId::References);

        SUBCASE("Constructor") {
            CHECK_NOTHROW(Node(serverOrClient, NodeId(0, UA_NS0ID_BOOLEAN), false));
            CHECK_NOTHROW(Node(serverOrClient, NodeId(0, UA_NS0ID_BOOLEAN), true));
            CHECK_NOTHROW(Node(serverOrClient, NodeId(0, "DoesNotExist"), false));
            CHECK_THROWS(Node(serverOrClient, NodeId(0, "DoesNotExist"), true));
        }

        SUBCASE("getConnection") {
            CHECK(rootNode.getConnection() == serverOrClient);
        }

        SUBCASE("Node class of default nodes") {
            CHECK(serverOrClient.getRootNode().readNodeClass() == NodeClass::Object);
            CHECK(serverOrClient.getObjectsNode().readNodeClass() == NodeClass::Object);
            CHECK(serverOrClient.getTypesNode().readNodeClass() == NodeClass::Object);
            CHECK(serverOrClient.getViewsNode().readNodeClass() == NodeClass::Object);
        }

        SUBCASE("Add non-type nodes") {
            CHECK(objNode.addObject({1, 1000}, "object").readNodeClass() == NodeClass::Object);
            CHECK(objNode.addFolder({1, 1001}, "folder").readNodeClass() == NodeClass::Object);
            CHECK(
                objNode.addVariable({1, 1002}, "variable").readNodeClass() == NodeClass::Variable
            );
            CHECK(
                objNode.addProperty({1, 1003}, "property").readNodeClass() == NodeClass::Variable
            );
#ifdef UA_ENABLE_METHODCALLS
            CHECK(
                objNode.addMethod({1, 1004}, "method", {}, {}, {}).readNodeClass() ==
                NodeClass::Method
            );
#endif
        }

        SUBCASE("Add type nodes") {
            CHECK(
                serverOrClient.getNode(ObjectTypeId::BaseObjectType)
                    .addObjectType({1, 1000}, "objecttype")
                    .readNodeClass() == NodeClass::ObjectType
            );
            CHECK(
                serverOrClient.getNode(VariableTypeId::BaseVariableType)
                    .addVariableType({1, 1001}, "variabletype")
                    .readNodeClass() == NodeClass::VariableType
            );
        }

        SUBCASE("Delete node") {
            auto node = objNode.addObject({1, 1000}, "object");
            CHECK_NOTHROW(node.deleteNode());
        }

        SUBCASE("Browse references") {
            const auto refs = rootNode.browseReferences();
            CHECK(refs.size() > 0);
            CHECK(std::any_of(refs.begin(), refs.end(), [&](auto& ref) {
                return ref.getBrowseName() == QualifiedName(0, "Objects");
            }));
        }

        SUBCASE("Browse referenced nodes") {
            const auto nodes = objNode.browseReferencedNodes();
            CHECK(nodes.size() > 0);
            CHECK(std::any_of(nodes.begin(), nodes.end(), [&](auto& node) {
                return node == rootNode;
            }));
        }

        SUBCASE("Browse children") {
            CHECK(rootNode.browseChildren(ReferenceTypeId::HasChild).empty());

            const auto nodes = rootNode.browseChildren(ReferenceTypeId::HierarchicalReferences);
            CHECK(nodes.size() > 0);
            CHECK(std::any_of(nodes.begin(), nodes.end(), [&](auto& node) {
                return node == objNode;
            }));
        }

        SUBCASE("Browse child") {
            CHECK_THROWS_WITH(rootNode.browseChild({{0, "Invalid"}}), "BadNoMatch");
            CHECK_EQ(
                rootNode.browseChild({{0, "Objects"}}).getNodeId(),
                NodeId(0, UA_NS0ID_OBJECTSFOLDER)
            );
            CHECK_EQ(
                rootNode.browseChild({{0, "Objects"}, {0, "Server"}}).getNodeId(),
                NodeId(0, UA_NS0ID_SERVER)
            );
        }

        SUBCASE("Browse parent") {
            CHECK_THROWS_WITH(rootNode.browseParent(), "BadNotFound");
            CHECK_EQ(objNode.browseParent(), rootNode);
        }

        SUBCASE("Read/write variable attributes") {
            CHECK_EQ(
                varNode.writeDisplayName({"en-US", "name"}).readDisplayName(),
                LocalizedText({"en-US", "name"})
            );
            CHECK_EQ(
                varNode.writeDescription({"en-US", "desc"}).readDescription(),
                LocalizedText({"en-US", "desc"})
            );
            CHECK_EQ(varNode.writeWriteMask(0xFFFFFFFF).readWriteMask(), 0xFFFFFFFF);
            CHECK_EQ(
                varNode.writeDataType(DataTypeId::Boolean).readDataType(),
                NodeId(DataTypeId::Boolean)
            );
            CHECK_EQ(
                varNode.template writeDataType<double>().readDataType(), NodeId(DataTypeId::Double)
            );
            CHECK_EQ(
                varNode.writeValueRank(ValueRank::TwoDimensions).readValueRank(),
                ValueRank::TwoDimensions
            );
            CHECK_EQ(
                varNode.writeArrayDimensions({2, 3}).readArrayDimensions(),
                std::vector<uint32_t>{2, 3}
            );
            CHECK_EQ(varNode.writeAccessLevel(0xFF).readAccessLevel(), 0xFF);
            CHECK_EQ(
                varNode.writeMinimumSamplingInterval(11.11).readMinimumSamplingInterval(), 11.11
            );
        }

        SUBCASE("Read/write value") {
            SUBCASE("Try read/write node classes other than Variable") {
                CHECK_THROWS(rootNode.template readScalar<int>());
                CHECK_THROWS(rootNode.template writeScalar<int>({}));
            }

            SUBCASE("Scalar") {
                CHECK_NOTHROW(varNode.writeDataType(Type::Float));

                // write with wrong data type
                CHECK_THROWS(varNode.writeScalar(bool{}));
                CHECK_THROWS(varNode.writeScalar(int{}));

                // write with correct data type
                float value = 11.11f;
                CHECK_NOTHROW(varNode.writeScalar(value));
                CHECK(varNode.template readScalar<float>() == value);
            }

            SUBCASE("String") {
                CHECK_NOTHROW(varNode.writeDataType(Type::String));

                String str("test");
                CHECK_NOTHROW(varNode.writeScalar(str));
                CHECK(varNode.template readScalar<std::string>() == "test");
            }

            SUBCASE("Array") {
                CHECK_NOTHROW(varNode.writeDataType(Type::Double));

                // write with wrong data type
                CHECK_THROWS(varNode.writeArray(std::vector<int>{}));
                CHECK_THROWS(varNode.writeArray(std::vector<float>{}));

                // write with correct data type
                std::vector<double> array{11.11, 22.22, 33.33};

                SUBCASE("Write as std::vector") {
                    CHECK_NOTHROW(varNode.writeArray(array));
                    CHECK(varNode.template readArray<double>() == array);
                }

                SUBCASE("Write as raw array") {
                    CHECK_NOTHROW(varNode.writeArray(Span{array.data(), array.size()}));
                    CHECK(varNode.template readArray<double>() == array);
                }

                SUBCASE("Write as iterator pair") {
                    CHECK_NOTHROW(varNode.writeArray(array.begin(), array.end()));
                    CHECK(varNode.template readArray<double>() == array);
                }
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
