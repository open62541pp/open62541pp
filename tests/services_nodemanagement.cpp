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
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addObjectAsync(
                connection,
                objectsId,
                newId,
                "Object",
                ObjectAttributes{},
                ObjectTypeId::BaseObjectType,
                ReferenceTypeId::HasComponent,
                useFuture
            );
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addObject(
                connection,
                objectsId,
                newId,
                "Object",
                ObjectAttributes{},
                ObjectTypeId::BaseObjectType,
                ReferenceTypeId::HasComponent
            );
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::Object);
    }

    SUBCASE("addFolder") {
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addFolderAsync(
                connection, objectsId, newId, "Folder", {}, ReferenceTypeId::HasComponent, useFuture
            );
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addFolder(
                connection, objectsId, newId, "Folder", {}, ReferenceTypeId::HasComponent
            );
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::Object);
    }

    SUBCASE("addVariable") {
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addVariableAsync(
                connection,
                objectsId,
                newId,
                "Variable",
                VariableAttributes{},
                VariableTypeId::BaseDataVariableType,
                ReferenceTypeId::HasComponent,
                useFuture
            );
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addVariable(
                connection,
                objectsId,
                newId,
                "Variable",
                VariableAttributes{},
                VariableTypeId::BaseDataVariableType,
                ReferenceTypeId::HasComponent
            );
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::Variable);
    }

    SUBCASE("addProperty") {
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addPropertyAsync(
                connection, objectsId, newId, "Property", {}, useFuture
            );
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addProperty(connection, objectsId, newId, "Property", {});
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::Variable);
    }

#ifdef UA_ENABLE_METHODCALLS
    SUBCASE("addMethod") {
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addMethodAsync(
                connection,
                objectsId,
                newId,
                "Method",
                {},  // callback
                {},  // input arguments
                {},  // output arguments
                MethodAttributes{},
                ReferenceTypeId::HasComponent,
                useFuture
            );
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addMethod(
                connection,
                objectsId,
                newId,
                "Method",
                {},  // callback
                {},  // input arguments
                {},  // output arguments
                MethodAttributes{},
                ReferenceTypeId::HasComponent
            );
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::Method);
    }
#endif

    SUBCASE("addObjectType") {
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addObjectTypeAsync(
                connection,
                {0, UA_NS0ID_BASEOBJECTTYPE},
                newId,
                "ObjectType",
                ObjectTypeAttributes{},
                ReferenceTypeId::HasSubtype,
                useFuture
            );
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addObjectType(
                connection,
                {0, UA_NS0ID_BASEOBJECTTYPE},
                newId,
                "ObjectType",
                ObjectTypeAttributes{},
                ReferenceTypeId::HasSubtype
            );
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::ObjectType);
    }

    SUBCASE("addVariableType") {
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addVariableTypeAsync(
                connection,
                {0, UA_NS0ID_BASEVARIABLETYPE},
                newId,
                "VariableType",
                VariableTypeAttributes{},
                VariableTypeId::BaseDataVariableType,
                ReferenceTypeId::HasSubtype,
                useFuture
            );
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addVariableType(
                connection,
                {0, UA_NS0ID_BASEVARIABLETYPE},
                newId,
                "VariableType",
                VariableTypeAttributes{},
                VariableTypeId::BaseDataVariableType,
                ReferenceTypeId::HasSubtype
            );
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::VariableType);
    }

    SUBCASE("addReferenceType") {
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addReferenceTypeAsync(
                connection,
                {0, UA_NS0ID_ORGANIZES},
                newId,
                "ReferenceType",
                ReferenceTypeAttributes{},
                ReferenceTypeId::HasSubtype,
                useFuture
            );
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addReferenceType(
                connection,
                {0, UA_NS0ID_ORGANIZES},
                newId,
                "ReferenceType",
                ReferenceTypeAttributes{},
                ReferenceTypeId::HasSubtype
            );
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::ReferenceType);
    }

    SUBCASE("addDataType") {
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addDataTypeAsync(
                connection,
                {0, UA_NS0ID_STRUCTURE},
                newId,
                "DataType",
                DataTypeAttributes{},
                ReferenceTypeId::HasSubtype,
                useFuture
            );
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addDataType(
                connection,
                {0, UA_NS0ID_STRUCTURE},
                newId,
                "DataType",
                DataTypeAttributes{},
                ReferenceTypeId::HasSubtype
            );
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::DataType);
    }

    SUBCASE("addView") {
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addViewAsync(
                connection,
                {0, UA_NS0ID_VIEWSFOLDER},
                newId,
                "View",
                ViewAttributes{},
                ReferenceTypeId::Organizes,
                useFuture
            );
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addView(
                connection,
                {0, UA_NS0ID_VIEWSFOLDER},
                newId,
                "View",
                ViewAttributes{},
                ReferenceTypeId::Organizes
            );
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::View);
    }

    SUBCASE("Add/delete reference") {
        services::addFolder(
            connection, objectsId, {1, 1000}, "Folder", {}, ReferenceTypeId::HasComponent
        )
            .value();
        services::addObject(
            connection,
            objectsId,
            {1, 1001},
            "Object",
            {},
            ObjectTypeId::BaseObjectType,
            ReferenceTypeId::HasComponent
        )
            .value();

        auto addReference = [&] {
            if constexpr (isAsync<T>) {
                auto future = services::addReferenceAsync(
                    connection, {1, 1000}, {1, 1001}, ReferenceTypeId::Organizes, true, useFuture
                );
                setup.client.runIterate();
                return future.get();
            } else {
                return services::addReference(
                    connection, {1, 1000}, {1, 1001}, ReferenceTypeId::Organizes, true
                );
            }
        };
        CHECK(addReference().isGood());
        CHECK(addReference() == UA_STATUSCODE_BADDUPLICATEREFERENCENOTALLOWED);

        auto deleteReference = [&] {
            if constexpr (isAsync<T>) {
                auto future = services::deleteReferenceAsync(
                    connection,
                    {1, 1000},
                    {1, 1001},
                    ReferenceTypeId::Organizes,
                    true,
                    true,
                    useFuture
                );
                setup.client.runIterate();
                return future.get();
            } else {
                return services::deleteReference(
                    connection, {1, 1000}, {1, 1001}, ReferenceTypeId::Organizes, true, true
                );
            }
        };
        CHECK(deleteReference().isGood());
    }

    SUBCASE("Delete node") {
        services::addObject(
            connection,
            objectsId,
            {1, 1000},
            "Object",
            {},
            ObjectTypeId::BaseObjectType,
            ReferenceTypeId::HasComponent
        )
            .value();

        auto deleteNode = [&] {
            if constexpr (isAsync<T>) {
                auto future = services::deleteNodeAsync(connection, {1, 1000}, true, useFuture);
                setup.client.runIterate();
                return future.get();
            } else {
                return services::deleteNode(connection, {1, 1000}, true);
            }
        };
        CHECK(deleteNode().isGood());
        CHECK(deleteNode() == UA_STATUSCODE_BADNODEIDUNKNOWN);
    }

    SUBCASE("Random node id") {
        // https://www.open62541.org/doc/1.3/server.html#node-addition-and-deletion
        const auto id =
            services::addObject(
                connection,
                objectsId,
                {1, 0},
                "Random",
                {},
                ObjectTypeId::BaseObjectType,
                ReferenceTypeId::HasComponent
            )
                .value();
        CHECK(id != NodeId(1, 0));
        CHECK(id.getNamespaceIndex() == 1);
    }
}
