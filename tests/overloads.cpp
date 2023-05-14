#include <sstream>

#include <doctest/doctest.h>

#include "open62541pp/overloads/ostream.h"
#include "open62541pp/types/Builtin.h"

using namespace opcua;

TEST_CASE("ostream overloads") {
    std::stringstream ss;

    SUBCASE("String") {
        String str("test123");
        ss << str;
        CHECK(ss.str() == "test123");
    }
    SUBCASE("Guid") {
        Guid guid{};
        ss << guid;
        CHECK(ss.str() == "00000000-0000-0000-0000-000000000000");
    }
}
