#include <cstring>
#include <stdexcept>

#include <doctest/doctest.h>

#include "open62541pp/detail/string_utils.h"

using namespace opcua;

TEST_CASE("UA_String from string_view") {
    SUBCASE("Test string") {
        std::string_view sv("test123");
        UA_String str = detail::toNativeString(sv);
        CHECK(str.length == sv.size());
        CHECK((void*)str.data == (void*)sv.data());
    }

    SUBCASE("Null string") {
        UA_String str = detail::toNativeString({});
        CHECK(str.length == 0);
        CHECK(str.data == nullptr);
    }

    SUBCASE("Empty string") {
        UA_String str = detail::toNativeString("");
        CHECK(str.length == 0);
        CHECK(str.data != nullptr);
    }
}

TEST_CASE("Alloc UA_String from string_view") {
    const char* cstr = "test123";
    std::string_view sv(cstr);
    auto str = detail::allocNativeString(sv);
    CHECK(str.length == 7);
    CHECK(std::strncmp((char*)str.data, sv.data(), 7) == 0);  // NOLINT
    UA_clear(&str, &UA_TYPES[UA_TYPES_STRING]);
}

TEST_CASE("Alloc UA_String from non-null-terminated string_view") {
    std::string str("test123");
    std::string_view sv(str.c_str(), 4);
    auto uaString = detail::allocNativeString(sv);
    CHECK(uaString.length == 4);
    CHECK(std::strncmp((char*)uaString.data, sv.data(), 4) == 0);  // NOLINT
    UA_clear(&uaString, &UA_TYPES[UA_TYPES_STRING]);
}

TEST_CASE("String conversion UA_String -> string") {
    SUBCASE("Test string") {
        UA_String str = UA_STRING_ALLOC("test123");
        CHECK(detail::toString(str) == "test123");
        UA_clear(&str, &UA_TYPES[UA_TYPES_STRING]);
    }

    SUBCASE("Null string") {
        UA_String str{};
        CHECK(detail::toString(str) == "");
    }

    SUBCASE("Empty string") {
        UA_String str = UA_STRING_ALLOC("");
        CHECK(detail::toString(str) == "");
        UA_clear(&str, &UA_TYPES[UA_TYPES_STRING]);
    }
}

TEST_CASE("String conversion format string and args -> string") {
    CHECK(detail::toString("test") == "test");
    CHECK(detail::toString("test %d", 11) == "test 11");
    CHECK(detail::toString("test %.2f", 11.11) == "test 11.11");
}
