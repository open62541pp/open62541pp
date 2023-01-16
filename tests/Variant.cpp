#include <array>
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
        REQUIRE(varEmpty.getVariantType() == std::nullopt);
        REQUIRE(varEmpty.getArrayLength() == 0);
        REQUIRE(varEmpty.getArrayDimensions().empty());

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
        }
    }

    SECTION("Set/get scalar") {
        Variant var;
        int32_t value = 5;
        var.setScalar(value);

        REQUIRE(var.isScalar());
        REQUIRE(var.isType(&UA_TYPES[UA_TYPES_INT32]));
        REQUIRE(var.isType(Type::Int32));
        REQUIRE(var.isType(NodeId{UA_NS0ID_INT32, 0}));
        REQUIRE(var.getVariantType().value() == Type::Int32);

        REQUIRE_THROWS(var.getScalar<bool>());
        REQUIRE_THROWS(var.getScalar<int16_t>());
        REQUIRE_THROWS(var.getArrayCopy<int32_t>());
        REQUIRE(var.getScalar<int32_t>() == value);
        REQUIRE(var.getScalarCopy<int32_t>() == value);
    }

    SECTION("Set/get scalar reference") {
        Variant var;
        int value = 3;
        var.setScalar(value);
        int& ref = var.getScalar<int>();
        REQUIRE(ref == value);
        REQUIRE(&ref == &value);

        value++;
        REQUIRE(ref == value);
        ref++;
        REQUIRE(ref == value);
    }

    SECTION("Set/get mixed scalar types") {
        Variant var;

        var.setScalarCopy(static_cast<int>(11));
        REQUIRE(var.getScalar<int>() == 11);
        REQUIRE(var.getScalarCopy<int>() == 11);

        var.setScalarCopy(static_cast<float>(11.11));
        REQUIRE(var.getScalar<float>() == 11.11f);
        REQUIRE(var.getScalarCopy<float>() == 11.11f);

        var.setScalarCopy(static_cast<short>(1));
        REQUIRE(var.getScalar<short>() == 1);
        REQUIRE(var.getScalarCopy<short>() == 1);
    }

    SECTION("Set/get wrapped scalar types") {
        Variant var;

        {
            TypeWrapper<int, Type::Int32> value(10);
            var.setScalar(value);
            REQUIRE(var.getScalar<int>() == 10);
            REQUIRE(var.getScalarCopy<int>() == 10);
        }

        {
            LocalizedText value("text", "en-US");
            var.setScalar(value);
            REQUIRE(var.getScalarCopy<LocalizedText>() == value);
        }
    }

    SECTION("Set/get array") {
        Variant var;
        std::vector<float> value{0, 1, 2, 3, 4, 5};
        var.setArrayCopy(value);

        REQUIRE(var.isArray());
        REQUIRE(var.isType(Type::Float));
        REQUIRE(var.isType(NodeId{UA_NS0ID_FLOAT, 0}));
        REQUIRE(var.getVariantType().value() == Type::Float);
        REQUIRE(var.getArrayLength() == value.size());
        REQUIRE(var.handle()->data != value.data());

        REQUIRE_THROWS(var.getArrayCopy<int32_t>());
        REQUIRE_THROWS(var.getArrayCopy<bool>());
        REQUIRE(var.getArrayCopy<float>() == value);
    }

    SECTION("Set/get array reference") {
        Variant var;
        std::vector<float> value{0, 1, 2};
        var.setArray(value);
        REQUIRE(var.getArrayCopy<float>() == value);

        std::vector<float> valueChanged({3, 4, 5});
        value.assign(valueChanged.begin(), valueChanged.end());
        REQUIRE(var.getArrayCopy<float>() == valueChanged);
    }

    SECTION("Set array of native strings") {
        Variant var;
        std::array array{
            detail::allocUaString("item1"),
            detail::allocUaString("item2"),
            detail::allocUaString("item3"),
        };

        var.setArray<UA_String, Type::String>(array.data(), array.size());
        REQUIRE(var.getArrayLength() == array.size());
        REQUIRE(var.handle()->data == array.data());

        UA_clear(&array[0], &UA_TYPES[UA_TYPES_STRING]);
        UA_clear(&array[1], &UA_TYPES[UA_TYPES_STRING]);
        UA_clear(&array[2], &UA_TYPES[UA_TYPES_STRING]);
    }

    SECTION("Set/get array of strings") {
        Variant var;
        std::vector<std::string> value{"a", "b", "c"};
        var.setArrayCopy<std::string, Type::String>(value);

        REQUIRE(var.isArray());
        REQUIRE(var.isType(Type::String));
        REQUIRE(var.isType(NodeId{UA_NS0ID_STRING, 0}));
        REQUIRE(var.getVariantType().value() == Type::String);

        REQUIRE_THROWS(var.getScalarCopy<std::string>());
        REQUIRE_THROWS(var.getArrayCopy<int32_t>());
        REQUIRE_THROWS(var.getArrayCopy<bool>());
        REQUIRE(var.getArrayCopy<std::string>() == value);
    }
}
