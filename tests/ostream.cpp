#include <sstream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "open62541pp/ostream.h"

using namespace Catch::Matchers;
using namespace opcua;

TEST_CASE("ostream") {
    std::stringstream ss;

    SECTION("String") {
        String str("test123");
        ss << str;
        REQUIRE_THAT(ss.str(), Equals("test123"));
    }
    SECTION("Guid") {
        Guid guid{};
        ss << guid;
        REQUIRE_THAT(ss.str(), Equals("00000000-0000-0000-0000-000000000000"));
    }
}
