#include "catch2/catch.hpp"

#include "open62541pp/Helper.h"

using namespace opcua;

TEST_CASE("String conversion", "[opcua]") {
    UA_String testString = UA_STRING_ALLOC("test123");
    REQUIRE(uaStringToString(testString) == "test123");
    REQUIRE(uaStringToString(testString) != "test321");
    UA_String_deleteMembers(&testString);

    SECTION("Null string") {
        UA_String nullString {};
        REQUIRE(uaStringToString(nullString).empty());
    }

    SECTION("Empty string") {
        UA_String emptyString = UA_STRING_ALLOC("");
        REQUIRE(uaStringToString(emptyString).empty());
        UA_String_deleteMembers(&emptyString);
    }
}