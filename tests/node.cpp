#include <algorithm>  // any_of

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "open62541pp/config.hpp"
#include "open62541pp/node.hpp"

#include "helper/server_client_setup.hpp"

using namespace opcua;

TEMPLATE_TEST_CASE("Node", "", Server, Client, Async<Client>) {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& connection = setup.instance<TestType>();

    [[maybe_unused]] const auto await = [&](auto future) {
        setup.client.runIterate();
        return future.get();
    };

    const NodeId varId{1, 1};
    REQUIRE(services::addVariable(
        setup.server,
        {0, UA_NS0ID_OBJECTSFOLDER},
        varId,
        "Variable",
        {},
        VariableTypeId::BaseDataVariableType,
        ReferenceTypeId::HasComponent
    ));
    REQUIRE(services::writeAccessLevel(setup.server, varId, 0xFF).isGood());
    REQUIRE(services::writeWriteMask(setup.server, varId, 0xFFFFFFFF).isGood());

    const NodeId objId{1, 2};
    REQUIRE(services::addObject(
        setup.server,
        {0, UA_NS0ID_OBJECTSFOLDER},
        objId,
        "Object",
        {},
        ObjectTypeId::BaseObjectType,
        ReferenceTypeId::HasComponent
    ));
    REQUIRE(services::writeWriteMask(setup.server, objId, 0xFFFFFFFF).isGood());

    const NodeId methodId{1, 3};
    REQUIRE(services::addMethod(
        setup.server,
        {0, UA_NS0ID_OBJECTSFOLDER},
        methodId,
        "Method",
        {},
        {},
        {},
        {},
        ReferenceTypeId::HasComponent
    ));
    REQUIRE(services::writeWriteMask(setup.server, methodId, 0xFFFFFFFF).isGood());

    const NodeId refId{1, 4};
    REQUIRE(services::addReferenceType(
        setup.server,
        {0, UA_NS0ID_REFERENCES},
        refId,
        "ReferenceType",
        {},
        ReferenceTypeId::HasSubtype
    ));
    REQUIRE(services::writeWriteMask(setup.server, refId, 0xFFFFFFFF).isGood());

    Node objNode(connection, objId);
    Node varNode(connection, varId);
    Node methodNode(connection, methodId);
    Node refNode(connection, refId);

    SECTION("connection") {
        CHECK(Node(connection, {}).connection() == connection);
    }

    SECTION("id") {
        const NodeId id{1, 1000};
        CHECK(Node(connection, id).id() == id);
    }

    SECTION("exists") {
        CHECK(Node(connection, NodeId(0, UA_NS0ID_OBJECTSFOLDER)).exists());
        CHECK_FALSE(Node(connection, NodeId(0, "DoesNotExist")).exists());
    }

    SECTION("addFolder") {
        const NodeId id{1, 1000};
        if constexpr (isAsync<TestType>) {
            CHECK_NOTHROW(await(objNode.addFolderAsync(id, "Folder")).value());
        } else {
            CHECK_NOTHROW(objNode.addFolder(id, "Folder"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::Object);
    }

    SECTION("addObject") {
        const NodeId id{1, 1000};
        if constexpr (isAsync<TestType>) {
            CHECK_NOTHROW(await(objNode.addObjectAsync(id, "Object")).value());
        } else {
            CHECK_NOTHROW(objNode.addObject(id, "Object"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::Object);
    }

    SECTION("addVariable") {
        const NodeId id{1, 1000};
        if constexpr (isAsync<TestType>) {
            CHECK_NOTHROW(await(objNode.addVariableAsync(id, "Variable")).value());
        } else {
            CHECK_NOTHROW(objNode.addVariable(id, "Variable"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::Variable);
    }

    SECTION("addProperty") {
        const NodeId id{1, 1000};
        if constexpr (isAsync<TestType>) {
            CHECK_NOTHROW(await(objNode.addPropertyAsync(id, "Property")).value());
        } else {
            CHECK_NOTHROW(objNode.addProperty(id, "Property"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::Variable);
    }

#ifdef UA_ENABLE_METHODCALLS
    SECTION("addMethod") {
        const NodeId id{1, 1000};
        if constexpr (isAsync<TestType>) {
            CHECK_NOTHROW(await(objNode.addMethodAsync(id, "Method", {}, {}, {})).value());
        } else {
            CHECK_NOTHROW(objNode.addMethod(id, "Method", {}, {}, {}));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::Method);
    }
#endif

    SECTION("addObjectType") {
        Node parent(connection, ObjectTypeId::BaseObjectType);
        const NodeId id{1, 1000};
        if constexpr (isAsync<TestType>) {
            CHECK_NOTHROW(await(parent.addObjectTypeAsync(id, "ObjectType")).value());
        } else {
            CHECK_NOTHROW(parent.addObjectType(id, "ObjectType"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::ObjectType);
    }

    SECTION("addVariableType") {
        Node parent(connection, VariableTypeId::BaseVariableType);
        const NodeId id{1, 1000};
        if constexpr (isAsync<TestType>) {
            CHECK_NOTHROW(await(parent.addVariableTypeAsync(id, "VariableType")).value());
        } else {
            CHECK_NOTHROW(parent.addVariableType(id, "VariableType"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::VariableType);
    }

    SECTION("addReferenceType") {
        Node parent(connection, ReferenceTypeId::References);
        const NodeId id{1, 1000};
        if constexpr (isAsync<TestType>) {
            CHECK_NOTHROW(await(parent.addReferenceTypeAsync(id, "ReferenceType")).value());
        } else {
            CHECK_NOTHROW(parent.addReferenceType(id, "ReferenceType"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::ReferenceType);
    }

    SECTION("addDataType") {
        Node parent(connection, DataTypeId::BaseDataType);
        const NodeId id{1, 1000};
        if constexpr (isAsync<TestType>) {
            CHECK_NOTHROW(await(parent.addDataTypeAsync(id, "DataType")).value());
        } else {
            CHECK_NOTHROW(parent.addDataType(id, "DataType"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::DataType);
    }

    SECTION("addView") {
        Node parent(connection, ObjectId::ViewsFolder);
        const NodeId id{1, 1000};
        if constexpr (isAsync<TestType>) {
            CHECK_NOTHROW(await(parent.addViewAsync(id, "View")).value());
        } else {
            CHECK_NOTHROW(parent.addView(id, "View"));
        }
        CHECK(Node(connection, id).readNodeClass() == NodeClass::View);
    }

    SECTION("addReference/deleteReference") {
        auto folder = objNode.addFolder({1, 1000}, "Folder");
        auto object = objNode.addObject({1, 1001}, "Object");
        const NodeId referenceType(ReferenceTypeId::Organizes);

        // add
        if constexpr (isAsync<TestType>) {
            CHECK(await(folder.addReferenceAsync(object.id(), referenceType)).isGood());
        } else {
            CHECK_NOTHROW(folder.addReference(object.id(), referenceType));
        }

        // delete
        if constexpr (isAsync<TestType>) {
            CHECK(await(folder.deleteReferenceAsync(object.id(), referenceType)).isGood());
        } else {
            CHECK_NOTHROW(folder.deleteReference(object.id(), referenceType));
        }
    }

    SECTION("deleteNode") {
        if constexpr (isAsync<TestType>) {
            CHECK(await(objNode.deleteNodeAsync()).isGood());
        } else {
            CHECK_NOTHROW(objNode.deleteNode());
        }
    }

    SECTION("browseReferences") {
        Node root(connection, ObjectId::RootFolder);
        const auto refs = root.browseReferences();
        CHECK(refs.size() > 0);
        CHECK(std::any_of(refs.begin(), refs.end(), [&](auto& ref) {
            return ref.browseName() == QualifiedName(0, "Objects");
        }));
    }

    SECTION("browseReferencedNodes") {
        Node root(connection, ObjectId::RootFolder);
        Node child(connection, ObjectId::ObjectsFolder);
        const auto nodes = child.browseReferencedNodes();
        CHECK(nodes.size() > 0);
        CHECK(std::any_of(nodes.begin(), nodes.end(), [&](auto& node) { return node == root; }));
    }

    SECTION("browseChildren") {
        Node root(connection, ObjectId::RootFolder);
        Node child(connection, ObjectId::ObjectsFolder);
        CHECK(root.browseChildren(ReferenceTypeId::HasChild).empty());
        const auto nodes = root.browseChildren(ReferenceTypeId::HierarchicalReferences);
        CHECK(nodes.size() > 0);
        CHECK(std::any_of(nodes.begin(), nodes.end(), [&](auto& node) { return node == child; }));
    }

    SECTION("browseChild") {
        Node root(connection, ObjectId::RootFolder);
        CHECK_THROWS_WITH(root.browseChild({{0, "Invalid"}}), "BadNoMatch");
        CHECK(root.browseChild({{0, "Objects"}}).id() == NodeId(ObjectId::ObjectsFolder));
        CHECK(root.browseChild({{0, "Objects"}, {0, "Server"}}).id() == NodeId(ObjectId::Server));
    }

    SECTION("browseParent") {
        Node root(connection, ObjectId::RootFolder);
        Node child(connection, ObjectId::ObjectsFolder);
        CHECK_THROWS_WITH(root.browseParent(), "BadNotFound");
        CHECK(child.browseParent() == root);
    }

    SECTION("readNodeClass") {
        if constexpr (isAsync<TestType>) {
            CHECK(await(varNode.readNodeClassAsync()).value() == NodeClass::Variable);
        } else {
            CHECK(varNode.readNodeClass() == NodeClass::Variable);
        }
    }

    SECTION("readBrowseName") {
        const QualifiedName browseName(1, "Variable");
        if constexpr (isAsync<TestType>) {
            CHECK(await(varNode.readBrowseNameAsync()).value() == browseName);
        } else {
            CHECK(varNode.readBrowseName() == browseName);
        }
    }

    SECTION("writeDisplayName/readDisplayName") {
        // https://github.com/open62541/open62541/issues/6723
        const LocalizedText displayName({}, "DisplayName");
        if constexpr (isAsync<TestType>) {
            CHECK(await(varNode.writeDisplayNameAsync(displayName)).isGood());
            CHECK(await(varNode.readDisplayNameAsync()).value() == displayName);
        } else {
            CHECK_NOTHROW(varNode.writeDisplayName(displayName));
            CHECK(varNode.readDisplayName() == displayName);
        }
    }

    SECTION("writeDescription/readDescription") {
        const LocalizedText description("en-US", "Description");
        if constexpr (isAsync<TestType>) {
            CHECK(await(varNode.writeDescriptionAsync(description)).isGood());
            CHECK(await(varNode.readDescriptionAsync()).value() == description);
        } else {
            CHECK_NOTHROW(varNode.writeDescription(description));
            CHECK(varNode.readDescription() == description);
        }
    }

    SECTION("writeWriteMask/readWriteMask") {
        const auto writeMask = 0xFFFFFFFF;
        if constexpr (isAsync<TestType>) {
            CHECK(await(varNode.writeWriteMaskAsync(writeMask)).isGood());
            CHECK(await(varNode.readWriteMaskAsync()).value() == writeMask);
        } else {
            CHECK_NOTHROW(varNode.writeWriteMask(writeMask));
            CHECK(varNode.readWriteMask() == writeMask);
        }
    }

    SECTION("readUserWriteMask") {
        if constexpr (isAsync<TestType>) {
            CHECK_NOTHROW(await(varNode.readUserWriteMaskAsync()).value());
        } else {
            CHECK_NOTHROW(varNode.readUserWriteMask());
        }
    }

    SECTION("writeIsAbstract/readIsAbstract") {
        const bool isAbstract = true;
        if constexpr (isAsync<TestType>) {
            CHECK(await(refNode.writeIsAbstractAsync(isAbstract)).isGood());
            CHECK(await(refNode.readIsAbstractAsync()).value() == isAbstract);
        } else {
            CHECK_NOTHROW(refNode.writeIsAbstract(isAbstract));
            CHECK_NOTHROW(refNode.readIsAbstract());
        }
    }

    SECTION("writeSymmetric/readSymmetric") {
        const bool symmetric = true;
        if constexpr (isAsync<TestType>) {
            CHECK(await(refNode.writeSymmetricAsync(symmetric)).isGood());
            CHECK(await(refNode.readSymmetricAsync()).value() == symmetric);
        } else {
            CHECK_NOTHROW(refNode.writeSymmetric(symmetric));
            CHECK(refNode.readSymmetric() == symmetric);
        }
    }

    SECTION("writeInverseName/readInverseName") {
        const LocalizedText inverseName({}, "InverseName");
        if constexpr (isAsync<TestType>) {
            CHECK(await(refNode.writeInverseNameAsync(inverseName)).isGood());
            CHECK(await(refNode.readInverseNameAsync()).value() == inverseName);
        } else {
            CHECK_NOTHROW(refNode.writeInverseName(inverseName));
            CHECK(refNode.readInverseName() == inverseName);
        }
    }

    SECTION("writeEventNotifier/readEventNotifier") {
        const auto eventNotifier = EventNotifier::HistoryRead | EventNotifier::HistoryWrite;
        if constexpr (isAsync<TestType>) {
            CHECK(await(objNode.writeEventNotifierAsync(eventNotifier)).isGood());
            CHECK(await(objNode.readEventNotifierAsync()).value() == eventNotifier);
        } else {
            CHECK_NOTHROW(objNode.writeEventNotifier(eventNotifier));
            CHECK(objNode.readEventNotifier() == eventNotifier);
        }
    }

    SECTION("writeValue/readValue") {
        const double value = 11.11;
        const auto variant = Variant(value);
        if constexpr (isAsync<TestType>) {
            CHECK(await(varNode.writeValueAsync(variant)).isGood());
            CHECK(await(varNode.readValueAsync()).value().template scalar<double>() == value);
        } else {
            CHECK_NOTHROW(varNode.writeValue(variant));
            CHECK(varNode.readValue().template scalar<double>() == value);
        }
    }

    SECTION("writeDataType/readDataType") {
        const NodeId dataType(DataTypeId::Float);
        if constexpr (isAsync<TestType>) {
            CHECK(await(varNode.writeDataTypeAsync(dataType)).isGood());
            CHECK(await(varNode.readDataTypeAsync()).value() == dataType);
        } else {
            CHECK_NOTHROW(varNode.writeDataType(dataType));
            CHECK(varNode.readDataType() == dataType);
        }
    }

    SECTION("writeValueRank/readValueRank") {
        const auto valueRank = ValueRank::Scalar;
        if constexpr (isAsync<TestType>) {
            CHECK(await(varNode.writeValueRankAsync(valueRank)).isGood());
            CHECK(await(varNode.readValueRankAsync()).value() == valueRank);
        } else {
            CHECK_NOTHROW(varNode.writeValueRank(valueRank));
            CHECK(varNode.readValueRank() == valueRank);
        }
    }

    SECTION("writeArrayDimensions/readArrayDimensions") {
        const std::vector<uint32_t> dimensions{11};
        varNode.writeValueRank(ValueRank::OneDimension);
        if constexpr (isAsync<TestType>) {
            CHECK(await(varNode.writeArrayDimensionsAsync(dimensions)).isGood());
            CHECK(await(varNode.readArrayDimensionsAsync()).value().at(0) == 11);
        } else {
            CHECK_NOTHROW(varNode.writeArrayDimensions(dimensions));
            CHECK(varNode.readArrayDimensions() == dimensions);
        }
    }

    SECTION("readAccessLevel") {
        if constexpr (isAsync<TestType>) {
            CHECK_NOTHROW(await(varNode.readAccessLevelAsync()).value());
        } else {
            CHECK_NOTHROW(varNode.readAccessLevel());
        }
    }

    SECTION("readUserAccessLevel") {
        if constexpr (isAsync<TestType>) {
            CHECK_NOTHROW(await(varNode.readUserAccessLevelAsync()).value());
        } else {
            CHECK_NOTHROW(varNode.readUserAccessLevel());
        }
    }

    SECTION("writeMinimumSamplingInterval/readMinimumSamplingInterval") {
        const double milliseconds = 11.11;
        if constexpr (isAsync<TestType>) {
            CHECK(await(varNode.writeMinimumSamplingIntervalAsync(milliseconds)).isGood());
            CHECK(await(varNode.readMinimumSamplingIntervalAsync()).value() == milliseconds);
        } else {
            CHECK_NOTHROW(varNode.writeMinimumSamplingInterval(milliseconds));
            CHECK(varNode.readMinimumSamplingInterval() == milliseconds);
        }
    }

    SECTION("writeHistorizing/readHistorizing") {
        const bool historizing = true;
        if constexpr (isAsync<TestType>) {
            CHECK(await(varNode.writeHistorizingAsync(historizing)).isGood());
            CHECK(await(varNode.readHistorizingAsync()).value() == historizing);
        } else {
            CHECK_NOTHROW(varNode.writeHistorizing(historizing));
            CHECK(varNode.readHistorizing() == historizing);
        }
    }

    SECTION("writeExecutable/readExecutable") {
        const bool executable = true;
        if constexpr (isAsync<TestType>) {
            CHECK(await(methodNode.writeExecutableAsync(executable)).isGood());
            CHECK(await(methodNode.readExecutableAsync()).value() == executable);
        } else {
            CHECK_NOTHROW(methodNode.writeExecutable(executable));
            CHECK(methodNode.readExecutable() == executable);
        }
    }

    SECTION("readUserExecutable") {
        if constexpr (isAsync<TestType>) {
            CHECK_NOTHROW(await(methodNode.readUserExecutableAsync()).value());
        } else {
            CHECK_NOTHROW(methodNode.readUserExecutable());
        }
    }

#if UAPP_OPEN62541_VER_GE(1, 1)
    SECTION("readDataTypeDefinition") {
        Node node(connection, DataTypeId::BuildInfo);
        if constexpr (isAsync<TestType>) {
            CHECK_NOTHROW(await(node.readDataTypeDefinitionAsync()).value());
        } else {
            CHECK_NOTHROW(node.readDataTypeDefinition());
        }
    }
#endif

    SECTION("writeObjectProperty/readObjectProperty") {
        objNode.addProperty(
            {1, 1001},
            "Property",
            VariableAttributes{}
                .setWriteMask(WriteMask::None)
                .setAccessLevel(AccessLevel::CurrentRead | AccessLevel::CurrentWrite)
                .setDataType<double>()
                .setValue(opcua::Variant{11.11})
        );
        CHECK(objNode.readObjectProperty({1, "Property"}).template scalar<double>() == 11.11);
        CHECK_NOTHROW(objNode.writeObjectProperty({1, "Property"}, Variant(22.22)));
        CHECK(objNode.readObjectProperty({1, "Property"}).template scalar<double>() == 22.22);
    }

    SECTION("Equality") {
        const NodeId id1{1, 1000};
        const NodeId id2{1, 1001};
        CHECK(Node(connection, id1) == Node(connection, id1));
        CHECK(Node(connection, id1) != Node(connection, id2));
    }
}
