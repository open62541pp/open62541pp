#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "open62541pp/config.hpp"
#include "open62541pp/services/attribute_highlevel.hpp"  // readNodeClass
#include "open62541pp/services/nodemanagement.hpp"

#include "helper/server_client_setup.hpp"

using namespace opcua;

TEMPLATE_TEST_CASE("NodeManagement service set", "", Server, Client, Async<Client>) {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& server = setup.server;
    auto& connection = setup.instance<TestType>();

    const NodeId objectsId{0, UA_NS0ID_OBJECTSFOLDER};
    const NodeId newId{1, 1000};

    SECTION("addObject") {
        const auto addObject = [&](auto&&... args) {
            if constexpr (isAsync<TestType>) {
                auto future = services::addObjectAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
            } else {
                return services::addObject(args...);
            }
        };
        const Result<NodeId> result = addObject(
            connection,
            objectsId,
            newId,
            "Object",
            ObjectAttributes{},
            ObjectTypeId::BaseObjectType,
            ReferenceTypeId::HasComponent
        );
        CHECK(result.value() == newId);
        CHECK(services::readNodeClass(server, newId).value() == NodeClass::Object);
    }

    SECTION("addFolder") {
        const auto addFolder = [&](auto&&... args) {
            if constexpr (isAsync<TestType>) {
                auto future = services::addFolderAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
            } else {
                return services::addFolder(args...);
            }
        };
        const Result<NodeId> result = addFolder(
            connection,
            objectsId,
            newId,
            "Folder",
            ObjectAttributes{},
            ReferenceTypeId::HasComponent
        );
        CHECK(result.value() == newId);
        CHECK(services::readNodeClass(server, newId).value() == NodeClass::Object);
    }

    SECTION("addVariable") {
        const auto addVariable = [&](auto&&... args) {
            if constexpr (isAsync<TestType>) {
                auto future = services::addVariableAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
            } else {
                return services::addVariable(args...);
            }
        };
        const Result<NodeId> result = addVariable(
            connection,
            objectsId,
            newId,
            "Variable",
            VariableAttributes{},
            VariableTypeId::BaseDataVariableType,
            ReferenceTypeId::HasComponent
        );
        CHECK(result.value() == newId);
        CHECK(services::readNodeClass(server, newId).value() == NodeClass::Variable);
    }

    SECTION("addProperty") {
        const auto addProperty = [&](auto&&... args) {
            if constexpr (isAsync<TestType>) {
                auto future = services::addPropertyAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
            } else {
                return services::addProperty(args...);
            }
        };
        const Result<NodeId> result = addProperty(
            connection, objectsId, newId, "Property", VariableAttributes{}
        );
        CHECK(result.value() == newId);
        CHECK(services::readNodeClass(server, newId).value() == NodeClass::Variable);
    }

#ifdef UA_ENABLE_METHODCALLS
    SECTION("addMethod") {
        const auto addMethod = [&](auto&&... args) {
            if constexpr (isAsync<TestType>) {
                auto future = services::addMethodAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
            } else {
                return services::addMethod(args...);
            }
        };
        const Result<NodeId> result = addMethod(
            connection,
            objectsId,
            newId,
            "Method",
            nullptr,  // callback
            opcua::Span<const opcua::Argument>{},  // input
            opcua::Span<const opcua::Argument>{},  // output
            MethodAttributes{},
            ReferenceTypeId::HasComponent
        );
        CHECK(result.value() == newId);
        CHECK(services::readNodeClass(server, newId).value() == NodeClass::Method);
    }
#endif

    SECTION("addObjectType") {
        const auto addObjectType = [&](auto&&... args) {
            if constexpr (isAsync<TestType>) {
                auto future = services::addObjectTypeAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
            } else {
                return services::addObjectType(args...);
            }
        };
        const Result<NodeId> result = addObjectType(
            connection,
            opcua::ObjectTypeId::BaseObjectType,
            newId,
            "ObjectType",
            ObjectTypeAttributes{},
            ReferenceTypeId::HasSubtype
        );
        CHECK(result.value() == newId);
        CHECK(services::readNodeClass(server, newId).value() == NodeClass::ObjectType);
    }

    SECTION("addVariableType") {
        const auto addVariableType = [&](auto&&... args) {
            if constexpr (isAsync<TestType>) {
                auto future = services::addVariableTypeAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
            } else {
                return services::addVariableType(args...);
            }
        };
        const Result<NodeId> result = addVariableType(
            connection,
            VariableTypeId::BaseVariableType,
            newId,
            "VariableType",
            VariableTypeAttributes{},
            VariableTypeId::BaseDataVariableType,
            ReferenceTypeId::HasSubtype
        );
        CHECK(result.value() == newId);
        CHECK(services::readNodeClass(server, newId).value() == NodeClass::VariableType);
    }

    SECTION("addReferenceType") {
        const auto addReferenceType = [&](auto&&... args) {
            if constexpr (isAsync<TestType>) {
                auto future = services::addReferenceTypeAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
            } else {
                return services::addReferenceType(args...);
            }
        };
        const Result<NodeId> result = addReferenceType(
            connection,
            ReferenceTypeId::Organizes,
            newId,
            "ReferenceType",
            ReferenceTypeAttributes{},
            ReferenceTypeId::HasSubtype
        );
        CHECK(result.value() == newId);
        CHECK(services::readNodeClass(server, newId).value() == NodeClass::ReferenceType);
    }

    SECTION("addDataType") {
        const auto addDataType = [&](auto&&... args) {
            if constexpr (isAsync<TestType>) {
                auto future = services::addDataTypeAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
            } else {
                return services::addDataType(args...);
            }
        };
        const Result<NodeId> result = addDataType(
            connection,
            DataTypeId::Structure,
            newId,
            "DataType",
            DataTypeAttributes{},
            ReferenceTypeId::HasSubtype
        );
        CHECK(result.value() == newId);
        CHECK(services::readNodeClass(server, newId).value() == NodeClass::DataType);
    }

    SECTION("addView") {
        const auto addView = [&](auto&&... args) {
            if constexpr (isAsync<TestType>) {
                auto future = services::addViewAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
            } else {
                return services::addView(args...);
            }
        };
        const Result<NodeId> result = addView(
            connection,
            ObjectId::ViewsFolder,
            newId,
            "View",
            ViewAttributes{},
            ReferenceTypeId::Organizes
        );
        CHECK(result.value() == newId);
        CHECK(services::readNodeClass(server, newId).value() == NodeClass::View);
    }

    SECTION("Add/delete reference") {
        REQUIRE(
            services::addFolder(
                connection, objectsId, {1, 1000}, "Folder", {}, ReferenceTypeId::HasComponent
            )
        );
        REQUIRE(
            services::addObject(
                connection,
                objectsId,
                {1, 1001},
                "Object",
                {},
                ObjectTypeId::BaseObjectType,
                ReferenceTypeId::HasComponent
            )
        );

        const auto addReference = [&](auto&&... args) {
            if constexpr (isAsync<TestType>) {
                auto future = services::addReferenceAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
            } else {
                return services::addReference(args...);
            }
        };
        CHECK(
            addReference(
                connection, NodeId(1, 1000), NodeId(1, 1001), ReferenceTypeId::Organizes, true
            ) == UA_STATUSCODE_GOOD
        );
        CHECK(
            addReference(
                connection, NodeId(1, 1000), NodeId(1, 1001), ReferenceTypeId::Organizes, true
            ) == UA_STATUSCODE_BADDUPLICATEREFERENCENOTALLOWED
        );

        const auto deleteReference = [&](auto&&... args) {
            if constexpr (isAsync<TestType>) {
                auto future = services::deleteReferenceAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
            } else {
                return services::deleteReference(args...);
            }
        };
        CHECK(
            deleteReference(
                connection, NodeId(1, 1000), NodeId(1, 1001), ReferenceTypeId::Organizes, true, true
            ) == UA_STATUSCODE_GOOD
        );
    }

    SECTION("Delete node") {
        REQUIRE(
            services::addObject(
                connection,
                objectsId,
                {1, 1000},
                "Object",
                {},
                ObjectTypeId::BaseObjectType,
                ReferenceTypeId::HasComponent
            )
        );

        const auto deleteNode = [&](auto&&... args) {
            if constexpr (isAsync<TestType>) {
                auto future = services::deleteNodeAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
            } else {
                return services::deleteNode(args...);
            }
        };
        CHECK(deleteNode(connection, NodeId(1, 1000), true) == UA_STATUSCODE_GOOD);
        CHECK(deleteNode(connection, NodeId(1, 1000), true) == UA_STATUSCODE_BADNODEIDUNKNOWN);
    }

    SECTION("Random node id") {
        // https://www.open62541.org/doc/1.3/server.html#node-addition-and-deletion
        const auto id =
            services::addObject(
                connection,
                objectsId,
                {1, 0},
                "Random",
                ObjectAttributes{},
                ObjectTypeId::BaseObjectType,
                ReferenceTypeId::HasComponent
            )
                .value();

        CHECK(id != NodeId(1, 0));
        CHECK(id.namespaceIndex() == 1);
    }
}
