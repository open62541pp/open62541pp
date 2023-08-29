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

        SUBCASE("getConnection") {
            CHECK(rootNode.getConnection() == serverOrClient);
        }

        SUBCASE("getNodeId") {
            const NodeId id(0, UA_NS0ID_OBJECTSFOLDER);
            CHECK(Node(serverOrClient, id).getNodeId() == id);
        }

        SUBCASE("exists") {
            CHECK(Node(serverOrClient, NodeId(0, UA_NS0ID_OBJECTSFOLDER)).exists());
            CHECK_FALSE(Node(serverOrClient, NodeId(0, "DoesNotExist")).exists());
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

        SUBCASE("Add/delete reference") {
            auto folder = objNode.addFolder({1, 1000}, "folder");
            auto object = objNode.addObject({1, 1001}, "object");
            CHECK_NOTHROW(folder.addReference({1, 1001}, ReferenceTypeId::Organizes, true));
            CHECK_NOTHROW(folder.deleteReference({1, 1001}, ReferenceTypeId::Organizes, true, true)
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
                CHECK_THROWS(rootNode.template readValueScalar<int>());
                CHECK_THROWS(rootNode.template writeValueScalar<int>({}));
            }

            SUBCASE("Scalar") {
                CHECK_NOTHROW(varNode.writeDataType(Type::Float));

                // write with wrong data type
                CHECK_THROWS(varNode.writeValueScalar(bool{}));
                CHECK_THROWS(varNode.writeValueScalar(int{}));

                // write with correct data type
                float value = 11.11f;
                CHECK_NOTHROW(varNode.writeValueScalar(value));
                CHECK(varNode.template readValueScalar<float>() == value);
            }

            SUBCASE("String") {
                CHECK_NOTHROW(varNode.writeDataType(Type::String));

                String str("test");
                CHECK_NOTHROW(varNode.writeValueScalar(str));
                CHECK(varNode.template readValueScalar<std::string>() == "test");
            }

            SUBCASE("Array") {
                CHECK_NOTHROW(varNode.writeDataType(Type::Double));

                // write with wrong data type
                CHECK_THROWS(varNode.writeValueArray(std::vector<int>{}));
                CHECK_THROWS(varNode.writeValueArray(std::vector<float>{}));

                // write with correct data type
                std::vector<double> array{11.11, 22.22, 33.33};

                SUBCASE("Write as std::vector") {
                    CHECK_NOTHROW(varNode.writeValueArray(array));
                    CHECK(varNode.template readValueArray<double>() == array);
                }

                SUBCASE("Write as raw array") {
                    CHECK_NOTHROW(varNode.writeValueArray(Span{array.data(), array.size()}));
                    CHECK(varNode.template readValueArray<double>() == array);
                }

                SUBCASE("Write as iterator pair") {
                    CHECK_NOTHROW(varNode.writeValueArray(array.begin(), array.end()));
                    CHECK(varNode.template readValueArray<double>() == array);
                }
            }
        }

        SUBCASE("Read/write object property") {
            auto node = objNode.addObject({1, 1000}, "Object");
            node.addProperty(
                {1, 1001},
                "Property",
                VariableAttributes{}
                    .setWriteMask(0xFFFFFFFF)
                    .setAccessLevel(0xFF)
                    .setDataType<double>()
                    .setValueScalar(11.11)
            );

            CHECK(node.readObjectProperty({1, "Property"}).template getScalar<double>() == 11.11);
            CHECK_NOTHROW(node.writeObjectProperty({1, "Property"}, Variant::fromScalar(22.22)));
            CHECK(node.readObjectProperty({1, "Property"}).template getScalar<double>() == 22.22);
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
