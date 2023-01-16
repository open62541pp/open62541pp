#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "open62541pp/TypeConverter.h"

using namespace opcua;

TEMPLATE_TEST_CASE(
    "TypeConverter native scalars",
    "",
    UA_SByte,
    UA_Byte,
    UA_Int16,
    UA_UInt16,
    UA_Int32,
    UA_UInt32,
    UA_Int64,
    UA_UInt64,
    UA_Float,
    UA_Double
) {
    const TestType src{11};

    SECTION("fromNative") {
        TestType dst{};
        TypeConverter<TestType>::fromNative(src, dst);
        REQUIRE(dst == src);
    }

    SECTION("toNative") {
        TestType dst{};
        TypeConverter<TestType>::toNative(src, dst);
        REQUIRE(dst == src);
    }
}

TEMPLATE_TEST_CASE("TypeConverter native strings", "", UA_String, UA_ByteString, UA_XmlElement) {
    TestType src = detail::allocUaString("Test123");
    TestType dst = detail::allocUaString("Overwrite me");

    TypeConverter<TestType>::fromNative(src, dst);
    REQUIRE(UA_String_equal(&src, &dst));
    REQUIRE(src.data != dst.data);

    UA_clear(&src, &UA_TYPES[UA_TYPES_STRING]);
    UA_clear(&dst, &UA_TYPES[UA_TYPES_STRING]);
}

TEST_CASE("TypeConverter std::string") {
    SECTION("fromNative") {
        UA_String src = detail::allocUaString("Test123");
        std::string dst;

        TypeConverter<std::string>::fromNative(src, dst);
        REQUIRE(dst == "Test123");

        UA_clear(&src, &UA_TYPES[UA_TYPES_STRING]);
    }

    SECTION("toNative") {
        std::string src{"Test123"};
        UA_String dst{};

        TypeConverter<std::string>::toNative(src, dst);
        REQUIRE(detail::toString(dst) == "Test123");

        UA_clear(&dst, &UA_TYPES[UA_TYPES_STRING]);
    }
}
