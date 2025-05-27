#include <string_view>
#include <utility>  // move

#include <catch2/catch_test_macros.hpp>

#include "open62541pp/config.hpp"
#include "open62541pp/datatype.hpp"
#include "open62541pp/types.hpp"

using namespace opcua;

static UA_DataTypeMember makeDataTypeMember(
    [[maybe_unused]] const char* memberName,
    const UA_DataType& memberType,
    uint8_t padding,
    bool isArray,
    [[maybe_unused]] bool isOptional
) noexcept {
    UA_DataTypeMember result{};
#if UAPP_HAS_TYPEDESCRIPTION
    result.memberName = memberName;
#endif
#if UAPP_OPEN62541_VER_GE(1, 3)
    result.memberType = &memberType;
#else
    result.memberTypeIndex = memberType.typeIndex;
    result.namespaceZero = memberType.typeId.namespaceIndex == 0;
#endif
    result.padding = padding;
    result.isArray = isArray;  // NOLINT
#if UAPP_OPEN62541_VER_GE(1, 1)
    result.isOptional = isOptional;  // NOLINT
#endif
    return result;
}

static UA_DataType makeDataType(
    [[maybe_unused]] const char* typeName,
    UA_NodeId typeId,
    UA_NodeId binaryEncodingId,
    uint16_t memSize,
    uint8_t typeKind,
    bool pointerFree,
    bool overlayable,
    uint32_t membersSize,
    UA_DataTypeMember* members
) noexcept {
    UA_DataType result{};
#if UAPP_HAS_TYPEDESCRIPTION
    result.typeName = typeName;
#endif
    result.typeId = typeId;
#if UAPP_OPEN62541_VER_GE(1, 2)
    result.binaryEncodingId = binaryEncodingId;
#else
    assert(binaryEncodingId.identifierType == UA_NODEIDTYPE_NUMERIC);
    result.binaryEncodingId = binaryEncodingId.identifier.numeric;  // NOLINT
#endif
    result.memSize = memSize;
    result.typeKind = typeKind;
    result.pointerFree = pointerFree;
    result.overlayable = overlayable;
    result.membersSize = membersSize;
    result.members = members;
    return result;
}

/* -------------------------------- Samples data type definition -------------------------------- */

struct Point {
    float x;
    float y;
    float z;
};

static UA_DataTypeMember pointMembers[3] = {
    makeDataTypeMember(
        "x",
        UA_TYPES[UA_TYPES_FLOAT],
        0,  // first member
        false,
        false
    ),
    makeDataTypeMember(
        "y",
        UA_TYPES[UA_TYPES_FLOAT],
        offsetof(Point, y) - offsetof(Point, x) - sizeof(float),
        false,
        false
    ),
    makeDataTypeMember(
        "z",
        UA_TYPES[UA_TYPES_FLOAT],
        offsetof(Point, z) - offsetof(Point, y) - sizeof(float),
        false,
        false
    ),
};

static const UA_DataType pointType = makeDataType(
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

static void checkEqual(const UA_DataTypeMember& member, const UA_DataTypeMember& expected) {
#if UAPP_OPEN62541_VER_GE(1, 3)
    CHECK(member.memberType == expected.memberType);  // NOLINT
#else
    CHECK(member.memberTypeIndex == expected.memberTypeIndex);  // NOLINT
    CHECK((bool)member.namespaceZero == (bool)expected.namespaceZero);  // NOLINT
#endif
    CHECK((uint8_t)member.padding == (uint8_t)expected.padding);  // NOLINT
    CHECK((bool)member.isArray == (bool)expected.isArray);  // NOLINT
#if UAPP_OPEN62541_VER_GE(1, 1)
    CHECK((bool)member.isOptional == (bool)expected.isOptional);  // NOLINT
#endif
}

static void checkEqual(const UA_DataType& dt, const UA_DataType& expected) {
#if UAPP_HAS_TYPEDESCRIPTION
    CHECK(std::string_view{dt.typeName} == std::string_view{expected.typeName});
#endif
    CHECK((dt.typeId == expected.typeId));
    CHECK((dt.binaryEncodingId == expected.binaryEncodingId));
    CHECK(dt.memSize == expected.memSize);
    CHECK(dt.typeKind == expected.typeKind);
    CHECK(dt.pointerFree == expected.pointerFree);
    CHECK(dt.overlayable == expected.overlayable);
    for (uint8_t i = 0; i < dt.membersSize; ++i) {
        checkEqual(dt.members[i], expected.members[i]);
    }
}

TEST_CASE("DataTypeMember") {
    const auto native = makeDataTypeMember("test123", UA_TYPES[UA_TYPES_FLOAT], 11, false, false);

    SECTION("Construct form native") {
        DataTypeMember member{native};
#if UAPP_HAS_TYPEDESCRIPTION
        CHECK(member.memberName() == "test123");
#endif
#if UAPP_OPEN62541_VER_GE(1, 3)
        CHECK(member.memberType() == &UA_TYPES[UA_TYPES_FLOAT]);
#endif
        CHECK(member.padding() == 11);
        CHECK_FALSE(member.isArray());
        CHECK_FALSE(member.isOptional());
    }

    SECTION("Copy constructor") {
        DataTypeMember m1{native};
        DataTypeMember m2{m1};
        checkEqual(m2, native);
    }

    SECTION("Copy assignment") {
        DataTypeMember m1{native};
        DataTypeMember m2;
        m2 = m1;
        checkEqual(m2, native);
    }

    SECTION("Move constructor") {
        DataTypeMember m1{native};
        DataTypeMember m2{std::move(m1)};
        checkEqual(m1, {});  // NOLINT
        checkEqual(m2, native);
    }

    SECTION("Move assignment") {
        DataTypeMember m1{native};
        DataTypeMember m2;
        m2 = std::move(m1);
        checkEqual(m1, {});  // NOLINT
        checkEqual(m2, native);
    }

    SECTION("Set methods") {
        DataTypeMember member;
        member.setMemberName("test123");
        member.setMemberType(&UA_TYPES[UA_TYPES_FLOAT]);
        member.setPadding(11);
        member.setIsArray(false);
        member.setIsOptional(false);
        checkEqual(member, native);
    }
}

TEST_CASE("DataType") {
    SECTION("Construct from native") {
        DataType dt{pointType};
        CHECK(dt.handle() != &pointType);
#if UAPP_HAS_TYPEDESCRIPTION
        CHECK(dt.typeName() == pointType.typeName);
#endif
        CHECK(dt.typeId() == NodeId{1, 1001});
        CHECK(dt.binaryEncodingId() == NodeId{1, 1});
        CHECK(dt.memSize() == sizeof(Point));
        CHECK(dt.typeKind() == UA_DATATYPEKIND_STRUCTURE);
        CHECK(dt.pointerFree() == true);
        CHECK(dt.overlayable() == false);
        CHECK(dt.members().size() == 3);
        CHECK((dt.members()[0] == pointMembers[0]));
        CHECK((dt.members()[1] == pointMembers[1]));
        CHECK((dt.members()[2] == pointMembers[2]));
    }

    SECTION("Copy constructor") {
        DataType dt1{pointType};
        DataType dt2{dt1};
        CHECK(dt2 == dt1);
    }

    SECTION("Copy assignment") {
        DataType dt1{pointType};
        DataType dt2;
        dt2 = dt1;
        CHECK(dt2 == dt1);
    }

    SECTION("Move constructor") {
        DataType dt1{pointType};
        DataType dt2{std::move(dt1)};
        CHECK(dt1 != pointType);
        CHECK(dt2 == pointType);
    }

    SECTION("Move assignment") {
        DataType dt1{pointType};
        DataType dt2;
        dt2 = std::move(dt1);
        CHECK(dt1 != pointType);
        CHECK(dt2 == pointType);
    }

    SECTION("Set methods") {
        DataType dt;
        dt.setTypeName("Point");
        dt.setTypeId({1, 1001});
        dt.setBinaryEncodingId({1, 1});
        dt.setMemSize(sizeof(Point));
        dt.setTypeKind(UA_DATATYPEKIND_STRUCTURE);
        dt.setPointerFree(true);
        dt.setOverlayable(false);
        dt.setMembers({
            asWrapper<DataTypeMember>(pointMembers[0]),
            asWrapper<DataTypeMember>(pointMembers[1]),
            asWrapper<DataTypeMember>(pointMembers[2]),
        });
        checkEqual(dt, pointType);
    }
}

TEST_CASE("DataTypeBuilder") {
    SECTION("Enum") {
        enum class TestEnum {};
        const auto dt = DataTypeBuilder<TestEnum>::createEnum("TestEnum", {1, 1}, {1, 1}).build();

        CHECK(dt.memSize() == sizeof(TestEnum));
        CHECK(dt.typeKind() == UA_DATATYPEKIND_ENUM);
        CHECK(dt.pointerFree() == true);
        CHECK(dt.members().empty());
    }

    SECTION("Struct") {
        const auto dt =
            DataTypeBuilder<Point>::createStructure("Point", {1, 1001}, {1, 1})
                .addField<&Point::x>("x")
                .addField<&Point::y>("y")
                .addField<&Point::z>("z")
                .build();

        checkEqual(dt, pointType);
    }

    SECTION("Struct with array") {
        struct Measurements {
            UA_String description;
            size_t measurementsSize;
            float* measurements;
        };

        UA_DataTypeMember measurementsMembers[2] = {
            makeDataTypeMember("description", UA_TYPES[UA_TYPES_STRING], 0, false, false),
            makeDataTypeMember("measurements", UA_TYPES[UA_TYPES_FLOAT], 0, true, false),
        };

        const UA_DataType measurementsType = makeDataType(
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

        const auto dt =
            DataTypeBuilder<Measurements>::createStructure("Measurements", {1, 1002}, {1, 2})
                .addField<&Measurements::description>("description", UA_TYPES[UA_TYPES_STRING])
                .addField<&Measurements::measurementsSize, &Measurements::measurements>(
                    "measurements"
                )
                .build();

        checkEqual(dt, measurementsType);
    }

    SECTION("Struct with optional fields") {
        struct Opt {
            int16_t a;
            float* b;
            float* c;
        };

        UA_DataTypeMember optMembers[3] = {
            makeDataTypeMember(
                "a",
                UA_TYPES[UA_TYPES_INT16],
                0,  // first member
                false,
                false
            ),
            makeDataTypeMember(
                "b",
                UA_TYPES[UA_TYPES_FLOAT],
                offsetof(Opt, b) - offsetof(Opt, a) - sizeof(int16_t),
                false,
                true
            ),
            makeDataTypeMember(
                "c",
                UA_TYPES[UA_TYPES_FLOAT],
                offsetof(Opt, c) - offsetof(Opt, b) - sizeof(float*),
                false,
                true
            ),
        };

        const UA_DataType optType = makeDataType(
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

        const auto dt =
            DataTypeBuilder<Opt>::createStructure("Opt", {1, 1003}, {1, 3})
                .addField<&Opt::a>("a")
                .addField<&Opt::b>("b")
                .addField<&Opt::c>("c")
                .build();

        checkEqual(dt, optType);
    }

    SECTION("Union") {
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
            makeDataTypeMember(
                "optionA", UA_TYPES[UA_TYPES_DOUBLE], offsetof(Uni, fields.optionA), false, false
            ),
            makeDataTypeMember(
                "optionB", UA_TYPES[UA_TYPES_STRING], offsetof(Uni, fields.optionB), false, false
            ),
        };

        const UA_DataType uniType = makeDataType(
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

        const auto dt =
            DataTypeBuilder<Uni>::createUnion("Uni", {1, 1004}, {1, 4})
                .addUnionField<&Uni::fields, double>("optionA")
                .addUnionField<&Uni::fields, UA_String>("optionB", UA_TYPES[UA_TYPES_STRING])
                .build();

        checkEqual(dt, uniType);
    }

    SECTION("Struct with wrapper type") {
        struct SNative {
            UA_Guid guid;
            int value;
        };

        struct SWrapper {
            Guid guid;
            int value;
        };

        const auto dtNative =
            DataTypeBuilder<SNative>::createStructure("S", {1, 1005}, {1, 5})
                .addField<&SNative::guid>("guid")
                .addField<&SNative::value>("value")
                .build();

        const auto dtWrapper =
            DataTypeBuilder<SWrapper>::createStructure("S", {1, 1005}, {1, 5})
                .addField<&SWrapper::guid>("guid")
                .addField<&SWrapper::value>("value")
                .build();

        checkEqual(dtNative, dtWrapper);
    }
}

TEST_CASE("findDataType") {
    SECTION("Builtin") {
        CHECK(findDataType(NodeId{0, 0}) == nullptr);

        const auto& dt = UA_TYPES[UA_TYPES_FLOAT];
        CHECK(findDataType(NodeId{dt.typeId}) == &dt);
    }

    SECTION("With custom") {
        UA_DataType dt1{};
        dt1.typeId = UA_NODEID_NUMERIC(1, 1001);
        UA_DataType dt2{};
        dt2.typeId = UA_NODEID_NUMERIC(1, 1002);
        UA_DataType dt3{};
        dt3.typeId = UA_NODEID_NUMERIC(1, 1003);
        UA_DataType dt4{};
        dt4.typeId = UA_NODEID_NUMERIC(1, 1004);
        UA_DataType dt5{};
        dt5.typeId = UA_NODEID_NUMERIC(1, 1005);

        const UA_DataType types1[2]{dt1, dt2};
        const UA_DataType types2[3]{dt3, dt4, dt5};
        const UA_DataTypeArray next{nullptr, 3, types2};
        const UA_DataTypeArray head{&next, 2, types1};

        CHECK(findDataType(NodeId{0, 0}, &head) == nullptr);
        CHECK(findDataType(NodeId{0, 0}, &next) == nullptr);
        CHECK(findDataType(NodeId{1, 1001}, &head) == &types1[0]);
        CHECK(findDataType(NodeId{1, 1001}, &next) == nullptr);
        CHECK(findDataType(NodeId{1, 1002}, &head) == &types1[1]);
        CHECK(findDataType(NodeId{1, 1003}, &head) == &types2[0]);
        CHECK(findDataType(NodeId{1, 1004}, &head) == &types2[1]);
        CHECK(findDataType(NodeId{1, 1005}, &head) == &types2[2]);
        CHECK(findDataType(NodeId{1, 1006}, &head) == nullptr);
    }
}
