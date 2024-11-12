#include <doctest/doctest.h>

#include <algorithm>  // any_of

#include "open62541pp/config.hpp"
#include "open62541pp/node.hpp"

#include "helper/server_client_setup.hpp"

using namespace opcua;

TEST_CASE_TEMPLATE("Node", T, Server, Client, Async<Client>) {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& connection = setup.getInstance<T>();

    const NodeId varId{1, 1};
    REQUIRE(services::addVariable(
        setup.server,
        {0, UA_NS0ID_OBJECTSFOLDER},
        varId,
        "variable",
        {},
        VariableTypeId::BaseDataVariableType,
        ReferenceTypeId::HasComponent
    ));
    // set all bits to 1 -> allow all
    REQUIRE(services::writeAccessLevel(setup.server, varId, 0xFF).isGood());
    REQUIRE(services::writeWriteMask(setup.server, varId, 0xFFFFFFFF).isGood());

    Node rootNode(connection, ObjectId::RootFolder);
    Node objNode(connection, ObjectId::ObjectsFolder);
    Node varNode(connection, varId);
    Node refNode(connection, ReferenceTypeId::References);

    SUBCASE("connection") {
        CHECK(Node(connection, {}).connection() == connection);
    }

    SUBCASE("id") {
        const NodeId id(0, UA_NS0ID_OBJECTSFOLDER);
        CHECK(Node(connection, id).id() == id);
    }

    SUBCASE("exists") {
        CHECK(Node(connection, NodeId(0, UA_NS0ID_OBJECTSFOLDER)).exists());
        CHECK_FALSE(Node(connection, NodeId(0, "DoesNotExist")).exists());
    }

    SUBCASE("addFolder") {
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            auto future = objNode.addFolderAsync(id, "Folder");
            setup.client.runIterate();
            CHECK_NOTHROW(future.get().value());
        } else {
            CHECK_NOTHROW(objNode.addFolder(id, "Folder"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::Object);
    }

    SUBCASE("addObject") {
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            auto future = objNode.addObjectAsync(id, "Object");
            setup.client.runIterate();
            CHECK_NOTHROW(future.get().value());
        } else {
            CHECK_NOTHROW(objNode.addObject(id, "Object"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::Object);
    }

    SUBCASE("addVariable") {
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            auto future = objNode.addVariableAsync(id, "Variable");
            setup.client.runIterate();
            CHECK_NOTHROW(future.get().value());
        } else {
            CHECK_NOTHROW(objNode.addVariable(id, "Variable"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::Variable);
    }

    SUBCASE("addProperty") {
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            auto future = objNode.addPropertyAsync(id, "Property");
            setup.client.runIterate();
            CHECK_NOTHROW(future.get().value());
        } else {
            CHECK_NOTHROW(objNode.addProperty(id, "Property"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::Variable);
    }

#ifdef UA_ENABLE_METHODCALLS
    SUBCASE("addMethod") {
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            auto future = objNode.addMethodAsync(id, "Method", {}, {}, {});
            setup.client.runIterate();
            CHECK_NOTHROW(future.get().value());
        } else {
            CHECK_NOTHROW(objNode.addMethod(id, "Method", {}, {}, {}));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::Method);
    }
#endif

    SUBCASE("addObjectType") {
        Node parent(connection, ObjectTypeId::BaseObjectType);
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            auto future = parent.addObjectTypeAsync(id, "ObjectType");
            setup.client.runIterate();
            CHECK_NOTHROW(future.get().value());
        } else {
            CHECK_NOTHROW(parent.addObjectType(id, "ObjectType"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::ObjectType);
    }

    SUBCASE("addVariableType") {
        Node parent(connection, VariableTypeId::BaseVariableType);
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            auto future = parent.addVariableTypeAsync(id, "VariableType");
            setup.client.runIterate();
            CHECK_NOTHROW(future.get().value());
        } else {
            CHECK_NOTHROW(parent.addVariableType(id, "VariableType"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::VariableType);
    }

    SUBCASE("addReferenceType") {
        Node parent(connection, ReferenceTypeId::References);
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            auto future = parent.addReferenceTypeAsync(id, "ReferenceType");
            setup.client.runIterate();
            CHECK_NOTHROW(future.get().value());
        } else {
            CHECK_NOTHROW(parent.addReferenceType(id, "ReferenceType"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::ReferenceType);
    }

    SUBCASE("addDataType") {
        Node parent(connection, DataTypeId::BaseDataType);
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            auto future = parent.addDataTypeAsync(id, "DataType");
            setup.client.runIterate();
            CHECK_NOTHROW(future.get().value());
        } else {
            CHECK_NOTHROW(parent.addDataType(id, "DataType"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::DataType);
    }

    SUBCASE("addView") {
        Node parent(connection, ObjectId::ViewsFolder);
        const NodeId id{1, 1000};
        if constexpr (isAsync<T>) {
            auto future = parent.addViewAsync(id, "View");
            setup.client.runIterate();
            CHECK_NOTHROW(future.get().value());
        } else {
            CHECK_NOTHROW(parent.addView(id, "View"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::View);
    }

    SUBCASE("addReference/deleteReference") {
        auto folder = objNode.addFolder({1, 1000}, "Folder");
        auto object = objNode.addObject({1, 1001}, "Object");

        // add
        if constexpr (isAsync<T>) {
            auto future = folder.addReferenceAsync(object.id(), ReferenceTypeId::Organizes);
            setup.client.runIterate();
            CHECK(future.get().isGood());
        } else {
            CHECK_NOTHROW(folder.addReference(object.id(), ReferenceTypeId::Organizes));
        }

        // delete
        if constexpr (isAsync<T>) {
            auto future = folder.deleteReferenceAsync(
                object.id(), ReferenceTypeId::Organizes, true, true
            );
            setup.client.runIterate();
            CHECK(future.get().isGood());
        } else {
            CHECK_NOTHROW(
                folder.deleteReference(object.id(), ReferenceTypeId::Organizes, true, true)
            );
        }
    }

    SUBCASE("deleteNode") {
        auto node = objNode.addObject({1, 1000}, "Object");

        if constexpr (isAsync<T>) {
            auto future = node.deleteNodeAsync();
            setup.client.runIterate();
            CHECK(future.get().isGood());
        } else {
            CHECK_NOTHROW(node.deleteNode());
        }
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
        CHECK(std::any_of(nodes.begin(), nodes.end(), [&](auto& node) { return node == rootNode; })
        );
    }

    SUBCASE("Browse children") {
        CHECK(rootNode.browseChildren(ReferenceTypeId::HasChild).empty());

        const auto nodes = rootNode.browseChildren(ReferenceTypeId::HierarchicalReferences);
        CHECK(nodes.size() > 0);
        CHECK(std::any_of(nodes.begin(), nodes.end(), [&](auto& node) { return node == objNode; }));
    }

    SUBCASE("Browse child") {
        CHECK_THROWS_WITH(rootNode.browseChild({{0, "Invalid"}}), "BadNoMatch");
        CHECK_EQ(rootNode.browseChild({{0, "Objects"}}).id(), NodeId(0, UA_NS0ID_OBJECTSFOLDER));
        CHECK_EQ(
            rootNode.browseChild({{0, "Objects"}, {0, "Server"}}).id(), NodeId(0, UA_NS0ID_SERVER)
        );
    }

    SUBCASE("Browse parent") {
        CHECK_THROWS_WITH(rootNode.browseParent(), "BadNotFound");
        CHECK_EQ(objNode.browseParent(), rootNode);
    }

    SUBCASE("Read/write variable node attributes") {
        // https://github.com/open62541/open62541/issues/6723
        CHECK_EQ(
            varNode.writeDisplayName({{}, "name"}).readDisplayName(), LocalizedText({{}, "name"})
        );
        CHECK_EQ(
            varNode.writeDescription({"en-US", "desc"}).readDescription(),
            LocalizedText({"en-US", "desc"})
        );
        CHECK_EQ(varNode.writeWriteMask(0xFFFFFFFF).readWriteMask(), 0xFFFFFFFF);
        CHECK_EQ(
            varNode.writeDataType(DataTypeId::Boolean).readDataType(), NodeId(DataTypeId::Boolean)
        );
        CHECK_EQ(
            varNode.template writeDataType<double>().readDataType(), NodeId(DataTypeId::Double)
        );
        CHECK_EQ(
            varNode.writeValueRank(ValueRank::TwoDimensions).readValueRank(),
            ValueRank::TwoDimensions
        );
        CHECK_EQ(
            varNode.writeArrayDimensions({2, 3}).readArrayDimensions(), std::vector<uint32_t>{2, 3}
        );
        CHECK_EQ(varNode.writeAccessLevel(0xFF).readAccessLevel(), uint8_t{0xFF});
        CHECK_EQ(varNode.writeMinimumSamplingInterval(11.11).readMinimumSamplingInterval(), 11.11);
        CHECK_EQ(varNode.writeHistorizing(true).readHistorizing(), true);
    }

    SUBCASE("Read/write value") {
        SUBCASE("Try read/write node classes other than Variable") {
            CHECK_THROWS(rootNode.template readValueScalar<int>());
            CHECK_THROWS(rootNode.template writeValueScalar<int>({}));
        }

        SUBCASE("Scalar") {
            CHECK_NOTHROW(varNode.writeDataType(DataTypeId::Float));

            // write with wrong data type
            CHECK_THROWS(varNode.writeValueScalar(bool{}));
            CHECK_THROWS(varNode.writeValueScalar(int{}));

            // write with correct data type
            float value = 11.11f;
            CHECK_NOTHROW(varNode.writeValueScalar(value));
            CHECK(varNode.template readValueScalar<float>() == value);
        }

        SUBCASE("String") {
            CHECK_NOTHROW(varNode.writeDataType(DataTypeId::String));

            String str("test");
            CHECK_NOTHROW(varNode.writeValueScalar(str));
            CHECK(varNode.template readValueScalar<std::string>() == "test");
        }

        SUBCASE("Array") {
            CHECK_NOTHROW(varNode.writeDataType(DataTypeId::Double));

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
                .setWriteMask(WriteMask::None)
                .setAccessLevel(AccessLevel::CurrentRead | AccessLevel::CurrentWrite)
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
}
