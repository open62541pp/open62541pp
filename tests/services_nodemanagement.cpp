#include <doctest/doctest.h>

#include "open62541pp/config.hpp"
#include "open62541pp/services/attribute_highlevel.hpp"  // readNodeClass
#include "open62541pp/services/nodemanagement.hpp"

#include "helper/server_client_setup.hpp"

using namespace opcua;

TEST_CASE_TEMPLATE("NodeManagement service set", T, Server, Client, Async<Client>) {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& server = setup.server;
    auto& connection = setup.getInstance<T>();

    const NodeId objectsId{0, UA_NS0ID_OBJECTSFOLDER};
    const NodeId newId{1, 1000};

    SUBCASE("addObject") {
        const auto addObject = [&](auto&&... args) {
            if constexpr (isAsync<T>) {
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
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::Object);
    }

    SUBCASE("addFolder") {
        const auto addFolder = [&](auto&&... args) {
            if constexpr (isAsync<T>) {
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
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::Object);
    }

    SUBCASE("addVariable") {
        const auto addVariable = [&](auto&&... args) {
            if constexpr (isAsync<T>) {
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
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::Variable);
    }

    SUBCASE("addProperty") {
        const auto addProperty = [&](auto&&... args) {
            if constexpr (isAsync<T>) {
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
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::Variable);
    }

#ifdef UA_ENABLE_METHODCALLS
    SUBCASE("addMethod") {
        const auto addMethod = [&](auto&&... args) {
            if constexpr (isAsync<T>) {
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
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::Method);
    }
#endif

    SUBCASE("addObjectType") {
        const auto addObjectType = [&](auto&&... args) {
            if constexpr (isAsync<T>) {
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
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::ObjectType);
    }

    SUBCASE("addVariableType") {
        const auto addVariableType = [&](auto&&... args) {
            if constexpr (isAsync<T>) {
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
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::VariableType);
    }

    SUBCASE("addReferenceType") {
        const auto addReferenceType = [&](auto&&... args) {
            if constexpr (isAsync<T>) {
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
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::ReferenceType);
    }

    SUBCASE("addDataType") {
        const auto addDataType = [&](auto&&... args) {
            if constexpr (isAsync<T>) {
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
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::DataType);
    }

    SUBCASE("addView") {
        const auto addView = [&](auto&&... args) {
            if constexpr (isAsync<T>) {
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
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::View);
    }

    SUBCASE("Add/delete reference") {
        REQUIRE(services::addFolder(
            connection, objectsId, {1, 1000}, "Folder", {}, ReferenceTypeId::HasComponent
        ));
        REQUIRE(services::addObject(
            connection,
            objectsId,
            {1, 1001},
            "Object",
            {},
            ObjectTypeId::BaseObjectType,
            ReferenceTypeId::HasComponent
        ));

        const auto addReference = [&](auto&&... args) {
            if constexpr (isAsync<T>) {
                auto future = services::addReferenceAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
            } else {
                return services::addReference(args...);
            }
        };
        CHECK_EQ(
            addReference(
                connection, NodeId(1, 1000), NodeId(1, 1001), ReferenceTypeId::Organizes, true
            ),
            UA_STATUSCODE_GOOD
        );
        CHECK_EQ(
            addReference(
                connection, NodeId(1, 1000), NodeId(1, 1001), ReferenceTypeId::Organizes, true
            ),
            UA_STATUSCODE_BADDUPLICATEREFERENCENOTALLOWED
        );

        const auto deleteReference = [&](auto&&... args) {
            if constexpr (isAsync<T>) {
                auto future = services::deleteReferenceAsync(args..., useFuture);
                setup.client.runIterate();
                return future.get();
            } else {
                return services::deleteReference(args...);
            }
        };
        CHECK_EQ(
            deleteReference(
                connection, NodeId(1, 1000), NodeId(1, 1001), ReferenceTypeId::Organizes, true, true
            ),
            UA_STATUSCODE_GOOD
        );
    }

    SUBCASE("Delete node") {
        REQUIRE(services::addObject(
            connection,
            objectsId,
            {1, 1000},
            "Object",
            {},
            ObjectTypeId::BaseObjectType,
            ReferenceTypeId::HasComponent
        ));

        const auto deleteNode = [&](auto&&... args) {
            if constexpr (isAsync<T>) {
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

    SUBCASE("Random node id") {
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
