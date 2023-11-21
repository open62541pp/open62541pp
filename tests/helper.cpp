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

TEST_CASE("UA_String from string_view") {
    SUBCASE("Test string") {
        std::string_view sv("test123");
        UA_String str = detail::toUaString(sv);
        CHECK(str.length == sv.size());
        CHECK((void*)str.data == (void*)sv.data());
    }

    SUBCASE("Null string") {
        UA_String str = detail::toUaString({});
        CHECK(str.length == 0);
        CHECK(str.data == nullptr);
    }

    SUBCASE("Empty string") {
        UA_String str = detail::toUaString("");
        CHECK(str.length == 0);
        CHECK(str.data != nullptr);
    }
}

TEST_CASE("Alloc UA_String from string_view") {
    const char* cstr = "test123";
    std::string_view sv(cstr);
    auto str = detail::allocUaString(sv);
    CHECK(str.length == 7);
    CHECK(std::strncmp((char*)str.data, sv.data(), 7) == 0);  // NOLINT
    UA_String_clear(&str);
}

TEST_CASE("Alloc UA_String from non-null-terminated string_view") {
    std::string str("test123");
    std::string_view sv(str.c_str(), 4);
    auto uaString = detail::allocUaString(sv);
    CHECK(uaString.length == 4);
    CHECK(std::strncmp((char*)uaString.data, sv.data(), 4) == 0);  // NOLINT
    UA_String_clear(&uaString);
}

TEST_CASE("String conversion UA_String -> string") {
    SUBCASE("Test string") {
        UA_String str = UA_STRING_ALLOC("test123");
        CHECK(detail::toString(str) == "test123");
        UA_String_clear(&str);
    }

    SUBCASE("Null string") {
        UA_String str{};
        CHECK(detail::toString(str) == "");
    }

    SUBCASE("Empty string") {
        UA_String str = UA_STRING_ALLOC("");
        CHECK(detail::toString(str) == "");
        UA_String_clear(&str);
    }
}
