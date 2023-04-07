#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include "open62541pp/Server.h"
#include "open62541pp/services/NodeManagement.h"

using namespace opcua;

TEST_CASE("NodeManagement") {
    Server server;
    const NodeId objectsId{0, UA_NS0ID_OBJECTSFOLDER};

    SECTION("Non-type nodes") {
        services::addFolder(server, objectsId, {1, 1000}, "folder");
        services::addObject(server, objectsId, {1, 1001}, "object");
        services::addVariable(server, objectsId, {1, 1002}, "variable");
        services::addProperty(server, objectsId, {1, 1003}, "property");
    }

    SECTION("Type nodes") {
        services::addObjectType(server, {0, UA_NS0ID_BASEOBJECTTYPE}, {1, 1000}, "objecttype");
        services::addVariableType(
            server, {0, UA_NS0ID_BASEVARIABLETYPE}, {1, 1001}, "variabletype"
        );
    }

    SECTION("Add existing node id") {
        services::addObject(server, objectsId, {1, 1000}, "object1");
        REQUIRE_THROWS_WITH(
            services::addObject(server, objectsId, {1, 1000}, "object2"), "BadNodeIdExists"
        );
    }

    SECTION("Add reference") {
        services::addFolder(server, objectsId, {1, 1000}, "folder");
        services::addObject(server, objectsId, {1, 1001}, "object");
        services::addReference(server, {1, 1000}, {1, 1001}, ReferenceType::Organizes);
        REQUIRE_THROWS_WITH(
            services::addReference(server, {1, 1000}, {1, 1001}, ReferenceType::Organizes),
            "BadDuplicateReferenceNotAllowed"
        );
    }

    SECTION("Delete node") {
        services::addObject(server, objectsId, {1, 1000}, "object");
        services::deleteNode(server, {1, 1000});
        REQUIRE_THROWS_WITH(services::deleteNode(server, {1, 1000}), "BadNodeIdUnknown");
    }
}
