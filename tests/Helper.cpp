#include <cstring>

#include <catch2/catch_test_macros.hpp>
#include <open62541/types_generated_handling.h>  // UA_String_clear

#include "open62541pp/Helper.h"

using namespace opcua;

TEST_CASE("getUaDataType") {
    const auto* expected = &UA_TYPES[UA_TYPES_BOOLEAN];

    REQUIRE(detail::getUaDataType(Type::Boolean) == expected);

    auto nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BOOLEAN);
    REQUIRE(detail::getUaDataType(&nodeId) == expected);
}

TEST_CASE("String conversion UA_String -> string") {
    UA_String testString = UA_STRING_ALLOC("test123");
    REQUIRE(detail::toString(testString) == "test123");
    UA_String_clear(&testString);

    SECTION("Null string") {
        UA_String nullString{};
        REQUIRE(detail::toString(nullString).empty());
    }

    SECTION("Empty string") {
        UA_String emptyString = UA_STRING_ALLOC("");
        REQUIRE(detail::toString(emptyString).empty());
        UA_String_clear(&emptyString);
    }
}

TEST_CASE("Alloc UA_String from string") {
    std::string str("test123");
    auto uaString = detail::allocUaString(str);
    REQUIRE(uaString.length == 7);
    REQUIRE(std::strncmp((char*)uaString.data, str.c_str(), 7) == 0);  // NOLINT
    UA_String_clear(&uaString);
}

TEST_CASE("Alloc UA_String from string_view") {
    const char* str = "test123";
    std::string_view sv(str);
    auto uaString = detail::allocUaString(sv);
    REQUIRE(uaString.length == 7);
    REQUIRE(std::strncmp((char*)uaString.data, sv.data(), 7) == 0);  // NOLINT
    UA_String_clear(&uaString);
}

TEST_CASE("Alloc UA_String from non-null-terminated string_view") {
    std::string str("test123");
    std::string_view sv(str.c_str(), 4);
    auto uaString = detail::allocUaString(sv);
    REQUIRE(uaString.length == 4);
    REQUIRE(std::strncmp((char*)uaString.data, sv.data(), 4) == 0);  // NOLINT
    UA_String_clear(&uaString);
}
