#include <cstring>

#include <doctest/doctest.h>

#include "open62541_impl.h"  // UA_String_clear
#include "open62541pp/detail/helper.h"

using namespace opcua;

TEST_CASE("getUaDataType") {
    const auto* expected = &UA_TYPES[UA_TYPES_BOOLEAN];
    CHECK(&detail::getUaDataType(UA_TYPES_BOOLEAN) == expected);
    CHECK(&detail::getUaDataType(Type::Boolean) == expected);
    CHECK(&detail::getUaDataType<UA_TYPES_BOOLEAN>() == expected);
    CHECK(&detail::getUaDataType<Type::Boolean>() == expected);
}

TEST_CASE("findUaDataType") {
    CHECK(detail::findUaDataType(UA_NODEID_NULL) == nullptr);

    const auto nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BOOLEAN);
    CHECK(detail::findUaDataType(nodeId) == &UA_TYPES[UA_TYPES_BOOLEAN]);
}

TEST_CASE("String conversion UA_String -> string") {
    UA_String testString = UA_STRING_ALLOC("test123");
    CHECK(detail::toString(testString) == "test123");
    UA_String_clear(&testString);

    SUBCASE("Null string") {
        UA_String nullString{};
        CHECK(detail::toString(nullString) == "");
    }

    SUBCASE("Empty string") {
        UA_String emptyString = UA_STRING_ALLOC("");
        CHECK(detail::toString(emptyString) == "");
        UA_String_clear(&emptyString);
    }
}

TEST_CASE("Alloc UA_String from string") {
    std::string str("test123");
    auto uaString = detail::allocUaString(str);
    CHECK(uaString.length == 7);
    CHECK(std::strncmp((char*)uaString.data, str.c_str(), 7) == 0);  // NOLINT
    UA_String_clear(&uaString);
}

TEST_CASE("Alloc UA_String from string_view") {
    const char* str = "test123";
    std::string_view sv(str);
    auto uaString = detail::allocUaString(sv);
    CHECK(uaString.length == 7);
    CHECK(std::strncmp((char*)uaString.data, sv.data(), 7) == 0);  // NOLINT
    UA_String_clear(&uaString);
}

TEST_CASE("Alloc UA_String from non-null-terminated string_view") {
    std::string str("test123");
    std::string_view sv(str.c_str(), 4);
    auto uaString = detail::allocUaString(sv);
    CHECK(uaString.length == 4);
    CHECK(std::strncmp((char*)uaString.data, sv.data(), 4) == 0);  // NOLINT
    UA_String_clear(&uaString);
}
