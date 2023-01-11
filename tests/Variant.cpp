#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "open62541pp/NodeId.h"
#include "open62541pp/Variant.h"
#include "open62541pp/open62541.h"

using namespace opcua;

TEST_CASE("Variant") {
    SECTION("Empty variant") {
        Variant varEmpty;

        REQUIRE(varEmpty.isEmpty());
        REQUIRE_FALSE(varEmpty.isScalar());
        REQUIRE_FALSE(varEmpty.isArray());

        SECTION("Type checks") {
            REQUIRE_FALSE(varEmpty.isType(Type::Boolean));
            REQUIRE_FALSE(varEmpty.isType(Type::Int16));
            REQUIRE_FALSE(varEmpty.isType(Type::UInt16));
            REQUIRE_FALSE(varEmpty.isType(Type::Int32));
            REQUIRE_FALSE(varEmpty.isType(Type::UInt32));
            REQUIRE_FALSE(varEmpty.isType(Type::Int64));
            REQUIRE_FALSE(varEmpty.isType(Type::UInt64));
            REQUIRE_FALSE(varEmpty.isType(Type::Float));
            REQUIRE_FALSE(varEmpty.isType(Type::Double));
            // ...

            REQUIRE_FALSE(varEmpty.isType<bool>());
            REQUIRE_FALSE(varEmpty.isType<int16_t>());
            REQUIRE_FALSE(varEmpty.isType<uint16_t>());
            REQUIRE_FALSE(varEmpty.isType<int32_t>());
            REQUIRE_FALSE(varEmpty.isType<uint32_t>());
            REQUIRE_FALSE(varEmpty.isType<int64_t>());
            REQUIRE_FALSE(varEmpty.isType<uint64_t>());
            REQUIRE_FALSE(varEmpty.isType<float>());
            REQUIRE_FALSE(varEmpty.isType<double>());
            // ...
        }
    }

    SECTION("Write/read scalar") {
        Variant var;
        int32_t value = 5;
        var.setScalar(value);

        REQUIRE(var.isScalar());
        REQUIRE(var.isType(&UA_TYPES[UA_TYPES_INT32]));
        REQUIRE(var.isType(Type::Int32));
        REQUIRE(var.isType(NodeId{UA_NS0ID_INT32, 0}));
        REQUIRE(var.isType<int32_t>());

        REQUIRE_NOTHROW(var.readScalar<int32_t>());
        REQUIRE_THROWS(var.readArray<int32_t>());
        REQUIRE_THROWS(var.readScalar<bool>());
        REQUIRE_THROWS(var.readScalar<int16_t>(&UA_TYPES[UA_TYPES_INT32]));

        REQUIRE(var.readScalar<int32_t>() == value);
    }

    SECTION("Write/read mixed scalar types") {
        Variant var;

        var.setScalar(static_cast<int>(11));
        REQUIRE(var.readScalar<int>() == 11);

        var.setScalar(static_cast<float>(11.11));
        REQUIRE(var.readScalar<float>() == 11.11f);

        var.setScalar(static_cast<short>(1));
        REQUIRE(var.readScalar<short>() == 1);
    }

    SECTION("Write/read array") {
        Variant var;
        std::vector<float> value{0, 1, 2, 3, 4, 5};
        var.setArray(value);

        REQUIRE(var.isArray());
        REQUIRE(var.isType<float>());
        REQUIRE(var.isType(Type::Float));
        REQUIRE(var.isType(NodeId{UA_NS0ID_FLOAT, 0}));

        REQUIRE_NOTHROW(var.readArray<float>());
        REQUIRE_THROWS(var.readArray<int32_t>());
        REQUIRE_THROWS(var.readArray<bool>());

        REQUIRE(var.readArray<float>() == value);
    }

    SECTION("Write array no copy") {
        Variant var;
        std::vector<float> value{0, 1, 2};
        var.setArrayNoCopy(value);

        REQUIRE(var.readArray<float>() == value);

        std::vector<float> valueChanged({3, 4, 5});
        value.assign(valueChanged.begin(), valueChanged.end());

        REQUIRE(var.readArray<float>() == valueChanged);
    }

    SECTION("Write/read wrapped types") {
        Variant var;

        {
            TypeWrapper<int> value(10);
            var.setScalar(value);
            REQUIRE(var.readScalar<int>() == 10);
        }

        {
            String value("Test");
            var.setScalar(value);
            REQUIRE(var.readScalar<String>() == value);
        }
    }
}
