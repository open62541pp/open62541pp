#include <catch2/catch_test_macros.hpp>

#include "open62541pp/Types.h"

using namespace opcua;

TEST_CASE("getType") {
    STATIC_REQUIRE(detail::getType<UA_Boolean>() == Type::Boolean);
    STATIC_REQUIRE(detail::getType<UA_String>() == Type::String);
}

TEST_CASE("getUaDataType") {
    auto* expected = &UA_TYPES[UA_TYPES_BOOLEAN];

    REQUIRE(detail::getUaDataType<bool>() == expected);
    REQUIRE(detail::getUaDataType(Type::Boolean) == expected);

    auto nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BOOLEAN);
    REQUIRE(detail::getUaDataType(&nodeId) == expected);
}
