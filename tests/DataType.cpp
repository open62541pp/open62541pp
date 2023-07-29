#include <utility>  // move

#include <doctest/doctest.h>

#include "open62541pp/DataType.h"
#include "open62541pp/types/NodeId.h"

using namespace opcua;

/* -------------------------------- Samples data type definition -------------------------------- */

struct Point {
    float x;
    float y;
    float z;
};

static UA_DataTypeMember pointMembers[3] = {
    detail::createDataTypeMember(
        "x",
        UA_TYPES[UA_TYPES_FLOAT],
        0,  // first member
        false,
        false
    ),
    detail::createDataTypeMember(
        "y",
        UA_TYPES[UA_TYPES_FLOAT],
        offsetof(Point, y) - offsetof(Point, x) - sizeof(float),
        false,
        false
    ),
    detail::createDataTypeMember(
        "z",
        UA_TYPES[UA_TYPES_FLOAT],
        offsetof(Point, z) - offsetof(Point, y) - sizeof(float),
        false,
        false
    ),
};

static const UA_DataType pointType = {
#ifdef UA_ENABLE_TYPEDESCRIPTION
    "Point",
#endif
    {1, UA_NODEIDTYPE_NUMERIC, {1001}},
    {1, UA_NODEIDTYPE_NUMERIC, {1}},
    sizeof(Point),
    UA_DATATYPEKIND_STRUCTURE,
    true,
    false,
    3,
    pointMembers,
};

/* ---------------------------------------------------------------------------------------------- */

TEST_CASE("DataType") {
    SUBCASE("Construct from native") {
        DataType dt(pointType);
        CHECK(dt.handle() != &pointType);
#ifdef UA_ENABLE_TYPEDESCRIPTION
        CHECK(dt.getTypeName() == pointType.typeName);
#endif
        CHECK(dt.getTypeId() == NodeId(1, 1001));
        CHECK(dt.getBinaryEncodingId() == NodeId(1, 1));
        CHECK(dt.getMemSize() == sizeof(Point));
        CHECK(dt.getTypeKind() == UA_DATATYPEKIND_STRUCTURE);
        CHECK(dt.getPointerFree() == true);
        CHECK(dt.getOverlayable() == false);
        CHECK(dt.getMembersSize() == 3);
        CHECK(dt.getMembers().at(0) == pointMembers[0]);
        CHECK(dt.getMembers().at(1) == pointMembers[1]);
        CHECK(dt.getMembers().at(2) == pointMembers[2]);
    }

    SUBCASE("Construct from type index") {
        CHECK(DataType(UA_TYPES_ARGUMENT) == UA_TYPES[UA_TYPES_ARGUMENT]);
    }

    SUBCASE("Copy constructor / assignment") {
        DataType dt1(pointType);
        DataType dt2(dt1);
        CHECK(dt2 == dt1);
    }

    SUBCASE("Copy assignment") {
        DataType dt1(pointType);
        DataType dt2;
        dt2 = dt1;
        CHECK(dt2 == dt1);
    }

    SUBCASE("Move constructor") {
        DataType dt1(pointType);
        DataType dt2(std::move(dt1));
        CHECK(dt1 != pointType);
        CHECK(dt2 == pointType);
    }

    SUBCASE("Move assignment") {
        DataType dt1(pointType);
        DataType dt2;
        dt2 = std::move(dt1);
        CHECK(dt1 != pointType);
        CHECK(dt2 == pointType);
    }

    SUBCASE("Set methods") {
        DataType dt;
        dt.setTypeName("Point");
        dt.setTypeId({1, 1001});
        dt.setBinaryEncodingId({1, 1});
        dt.setMemSize(sizeof(Point));
        dt.setTypeKind(UA_DATATYPEKIND_STRUCTURE);
        dt.setPointerFree(true);
        dt.setOverlayable(false);
        dt.setMembers({pointMembers[0], pointMembers[1], pointMembers[2]});
        CHECK(dt == pointType);
    }
}
