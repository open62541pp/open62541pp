#include <string_view>
#include <utility>  // move

#include <doctest/doctest.h>

#include "open62541pp/Config.h"
#include "open62541pp/DataType.h"
#include "open62541pp/DataTypeBuilder.h"
#include "open62541pp/types/Builtin.h"
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

static const UA_DataType pointType = detail::createDataType(
    "Point",
    UA_NODEID_NUMERIC(1, 1001),
    UA_NODEID_NUMERIC(1, 1),
    sizeof(Point),
    UA_DATATYPEKIND_STRUCTURE,
    true,
    false,
    3,
    pointMembers
);

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
        CHECK(dt.getMembers().size() == 3);
        CHECK(dt.getMembers()[0] == pointMembers[0]);
        CHECK(dt.getMembers()[1] == pointMembers[1]);
        CHECK(dt.getMembers()[2] == pointMembers[2]);
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

static void checkDataTypeEqual(const DataType& dt, const UA_DataType& expected) {
#ifdef UA_ENABLE_TYPEDESCRIPTION
    CHECK(std::string_view(dt.getTypeName()) == std::string_view(expected.typeName));
#endif
    CHECK(dt.getTypeId() == expected.typeId);
#if UAPP_OPEN62541_VER_GE(1, 2)
    CHECK(dt.getBinaryEncodingId() == expected.binaryEncodingId);
#else
    CHECK(dt.getBinaryEncodingId().getIdentifierAs<uint32_t>() == expected.binaryEncodingId);
#endif
    CHECK(dt.getMemSize() == expected.memSize);
    CHECK(dt.getTypeKind() == expected.typeKind);
    CHECK(dt.getPointerFree() == static_cast<bool>(expected.pointerFree));
    CHECK(dt.getOverlayable() == static_cast<bool>(expected.overlayable));
    for (uint8_t i = 0; i < dt.getMembers().size(); ++i) {
        CAPTURE(i);
        auto m = dt.getMembers()[i];
#if UAPP_OPEN62541_VER_GE(1, 3)
        CHECK(m.memberType == expected.members[i].memberType);
#else
        CHECK(m.memberTypeIndex == expected.members[i].memberTypeIndex);
        CHECK((bool)m.namespaceZero == (bool)expected.members[i].namespaceZero);  // NOLINT
#endif
        CHECK((uint8_t)m.padding == (uint8_t)expected.members[i].padding);  // NOLINT
        CHECK((bool)m.isArray == (bool)expected.members[i].isArray);  // NOLINT
#if UAPP_OPEN62541_VER_GE(1, 1)
        CHECK((bool)m.isOptional == (bool)expected.members[i].isOptional);  // NOLINT
#endif
    }
}

TEST_CASE("DataTypeBuilder") {
    SUBCASE("Struct") {
        auto dt = DataTypeBuilder<Point>::createStructure("Point", {1, 1001}, {1, 1})
                      .addField<&Point::x>("x")
                      .addField<&Point::y>("y")
                      .addField<&Point::z>("z")
                      .build();

        checkDataTypeEqual(dt, pointType);
    }

    SUBCASE("Struct with array") {
        struct Measurements {
            UA_String description;
            size_t measurementsSize;
            float* measurements;
        };

        UA_DataTypeMember measurementsMembers[2] = {
            detail::createDataTypeMember("description", UA_TYPES[UA_TYPES_STRING], 0, false, false),
            detail::createDataTypeMember("measurements", UA_TYPES[UA_TYPES_FLOAT], 0, true, false),
        };

        const UA_DataType measurementsType = detail::createDataType(
            "Measurements",
            UA_NODEID_NUMERIC(1, 1002),
            UA_NODEID_NUMERIC(1, 2),
            sizeof(Measurements),
            UA_DATATYPEKIND_STRUCTURE,
            false,
            false,
            2,
            measurementsMembers
        );

        auto dt =
            DataTypeBuilder<Measurements>::createStructure("Measurements", {1, 1002}, {1, 2})
                .addField<&Measurements::description>("description", UA_TYPES[UA_TYPES_STRING])
                .addField<&Measurements::measurementsSize, &Measurements::measurements>(
                    "measurements"
                )
                .build();

        checkDataTypeEqual(dt, measurementsType);
    }

    SUBCASE("Struct with optional fields") {
        struct Opt {
            int16_t a;
            float* b;
            float* c;
        };

        UA_DataTypeMember optMembers[3] = {
            detail::createDataTypeMember(
                "a",
                UA_TYPES[UA_TYPES_INT16],
                0,  // first member
                false,
                false
            ),
            detail::createDataTypeMember(
                "b",
                UA_TYPES[UA_TYPES_FLOAT],
                offsetof(Opt, b) - offsetof(Opt, a) - sizeof(int16_t),
                false,
                true
            ),
            detail::createDataTypeMember(
                "c",
                UA_TYPES[UA_TYPES_FLOAT],
                offsetof(Opt, c) - offsetof(Opt, b) - sizeof(float*),
                false,
                true
            ),
        };

        const UA_DataType optType = detail::createDataType(
            "Opt",
            UA_NODEID_NUMERIC(1, 1003),
            UA_NODEID_NUMERIC(1, 3),
            sizeof(Opt),
            UA_DATATYPEKIND_OPTSTRUCT,
            false,
            false,
            3,
            optMembers
        );

        auto dt = DataTypeBuilder<Opt>::createStructure("Opt", {1, 1003}, {1, 3})
                      .addField<&Opt::a>("a")
                      .addField<&Opt::b>("b")
                      .addField<&Opt::c>("c")
                      .build();

        checkDataTypeEqual(dt, optType);
    }

    SUBCASE("Union") {
        enum UniSwitch {
            UA_UNISWITCH_NONE = 0,
            UA_UNISWITCH_OPTIONA = 1,
            UA_UNISWITCH_OPTIONB = 2
        };

        struct Uni {
            UniSwitch switchField;

            union Fields {
                double optionA;
                UA_String optionB;
            } fields;
        };

        static UA_DataTypeMember uniMembers[2] = {
            detail::createDataTypeMember(
                "optionA", UA_TYPES[UA_TYPES_DOUBLE], offsetof(Uni, fields.optionA), false, false
            ),
            detail::createDataTypeMember(
                "optionB", UA_TYPES[UA_TYPES_STRING], offsetof(Uni, fields.optionB), false, false
            ),
        };

        const UA_DataType uniType = detail::createDataType(
            "Uni",
            UA_NODEID_NUMERIC(1, 1004),
            UA_NODEID_NUMERIC(1, 4),
            sizeof(Uni),
            UA_DATATYPEKIND_UNION,
            false,
            false,
            2,
            uniMembers
        );

        auto dt = DataTypeBuilder<Uni>::createUnion("Uni", {1, 1004}, {1, 4})
                      .addUnionField<&Uni::fields, double>("optionA")
                      .addUnionField<&Uni::fields, UA_String>("optionB", UA_TYPES[UA_TYPES_STRING])
                      .build();

        checkDataTypeEqual(dt, uniType);
    }

    SUBCASE("Struct with wrapper type") {
        struct SNative {
            UA_Guid guid;
            int value;
        };

        struct SWrapper {
            Guid guid;
            int value;
        };

        auto dtNative =
            DataTypeBuilder<SNative>::createStructure("S", {1, 1005}, {1, 5})
                .addField<&SNative::guid>("guid")
                .addField<&SNative::value>("value")
                .build();

        auto dtWrapper =
            DataTypeBuilder<SWrapper>::createStructure("S", {1, 1005}, {1, 5})
                .addField<&SWrapper::guid>("guid")
                .addField<&SWrapper::value>("value")
                .build();

        CHECK(dtNative == dtWrapper);
    }
}
