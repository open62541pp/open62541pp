#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include "open62541pp/Server.h"
#include "open62541pp/services/services.h"

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

TEST_CASE("Attribute") {
    Server server;
    const NodeId objectsId{0, UA_NS0ID_OBJECTSFOLDER};

    SECTION("Read/write node attributes") {
        const NodeId id{1, "testAttributes"};
        services::addVariable(server, objectsId, id, "testAttributes");

        // read default attributes
        REQUIRE(services::readNodeClass(server, id) == NodeClass::Variable);
        REQUIRE(services::readBrowseName(server, id) == "testAttributes");
        // REQUIRE(services::readDisplayName(server, id) == LocalizedText("", "testAttributes"));
        REQUIRE(services::readDescription(server, id).getText().empty());
        REQUIRE(services::readDescription(server, id).getLocale().empty());
        REQUIRE(services::readWriteMask(server, id) == 0);
        REQUIRE(services::readDataType(server, id) == NodeId(0, UA_NS0ID_BASEDATATYPE));
        REQUIRE(services::readValueRank(server, id) == ValueRank::Any);
        REQUIRE(services::readArrayDimensions(server, id).empty());
        REQUIRE(services::readAccessLevel(server, id) == UA_ACCESSLEVELMASK_READ);

        // write new attributes
        REQUIRE_NOTHROW(services::writeDisplayName(server, id, {"en-US", "newDisplayName"}));
        REQUIRE_NOTHROW(services::writeDescription(server, id, {"de-DE", "newDescription"}));
        REQUIRE_NOTHROW(services::writeWriteMask(server, id, 11));
        REQUIRE_NOTHROW(services::writeDataType(server, id, NodeId{0, 2}));
        REQUIRE_NOTHROW(services::writeValueRank(server, id, ValueRank::TwoDimensions));
        REQUIRE_NOTHROW(services::writeArrayDimensions(server, id, {3, 2}));
        REQUIRE_NOTHROW(services::writeAccessLevel(
            server, id, UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE
        ));

        // read new attributes
        REQUIRE(services::readDisplayName(server, id) == LocalizedText("en-US", "newDisplayName"));
        REQUIRE(services::readDescription(server, id) == LocalizedText("de-DE", "newDescription"));
        REQUIRE(services::readWriteMask(server, id) == 11);
        REQUIRE(services::readDataType(server, id) == NodeId(0, 2));
        REQUIRE(services::readValueRank(server, id) == ValueRank::TwoDimensions);
        REQUIRE(services::readArrayDimensions(server, id).size() == 2);
        REQUIRE(services::readArrayDimensions(server, id).at(0) == 3);
        REQUIRE(services::readArrayDimensions(server, id).at(1) == 2);
        REQUIRE(
            services::readAccessLevel(server, id) ==
            (UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE)
        );
    }

    SECTION("Value rank and array dimension combinations") {
        const NodeId id{1, "testDimensions"};
        services::addVariable(server, objectsId, id, "testDimensions");

        SECTION("Unspecified dimension (ValueRank <= 0)") {
            auto valueRank = GENERATE(
                ValueRank::Any,
                ValueRank::Scalar,
                ValueRank::ScalarOrOneDimension,
                ValueRank::OneOrMoreDimensions
            );
            CAPTURE(valueRank);
            services::writeValueRank(server, id, valueRank);
            CHECK_NOTHROW(services::writeArrayDimensions(server, id, {}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1, 2}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1, 2, 3}));
        }

        SECTION("OneDimension") {
            services::writeValueRank(server, id, ValueRank::OneDimension);
            services::writeArrayDimensions(server, id, {1});
            CHECK_THROWS(services::writeArrayDimensions(server, id, {}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1, 2}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1, 2, 3}));
        }

        SECTION("TwoDimensions") {
            services::writeValueRank(server, id, ValueRank::TwoDimensions);
            services::writeArrayDimensions(server, id, {1, 2});
            CHECK_THROWS(services::writeArrayDimensions(server, id, {}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1, 2, 3}));
        }

        SECTION("ThreeDimensions") {
            services::writeValueRank(server, id, ValueRank::ThreeDimensions);
            services::writeArrayDimensions(server, id, {1, 2, 3});
            CHECK_THROWS(services::writeArrayDimensions(server, id, {}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1, 2}));
        }
    }

    SECTION("Read/write value") {
        const NodeId id{1, "testValue"};
        services::addVariable(server, objectsId, id, "testValue");

        Variant variantWrite;
        variantWrite.setScalarCopy(11.11);
        services::writeValue(server, id, variantWrite);

        Variant variantRead;
        services::readValue(server, id, variantRead);
        CHECK(variantRead.getScalar<double>() == 11.11);
    }

    SECTION("Read/write data value") {
        const NodeId id{1, "testDataValue"};
        services::addVariable(server, objectsId, id, "testDataValue");

        Variant variant;
        variant.setScalarCopy<int>(11);
        DataValue valueWrite(variant, {}, DateTime::now(), {}, 1, UA_STATUSCODE_GOOD);
        services::writeDataValue(server, id, valueWrite);

        DataValue valueRead;
        services::readDataValue(server, id, valueRead);

        CHECK(valueRead->hasValue);
        CHECK(valueRead->hasServerTimestamp);
        CHECK(valueRead->hasSourceTimestamp);
        CHECK(valueRead->hasServerPicoseconds);
        CHECK(valueRead->hasSourcePicoseconds);
        CHECK_FALSE(valueRead->hasStatus);  // doesn't contain error code on success

        CHECK(valueRead.getValue().value().getScalar<int>() == 11);
        CHECK(valueRead->sourceTimestamp == valueWrite->sourceTimestamp);
        CHECK(valueRead->sourcePicoseconds == valueWrite->sourcePicoseconds);
    }
}

TEST_CASE("Browse") {
    Server server;

    SECTION("Browse child") {
        const NodeId rootId{0, UA_NS0ID_ROOTFOLDER};

        REQUIRE_THROWS(services::browseChild(server, rootId, {}));
        REQUIRE_THROWS(services::browseChild(server, rootId, {{0, "Invalid"}}));
        REQUIRE(
            services::browseChild(server, rootId, {{0, "Types"}, {0, "ObjectTypes"}}) ==
            NodeId{0, UA_NS0ID_OBJECTTYPESFOLDER}
        );
    }
}
