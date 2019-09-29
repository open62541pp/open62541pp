#include <vector>

#include "catch2/catch.hpp"
#include "open62541pp/Variant.h"

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

            REQUIRE_FALSE(varEmpty.isType<bool>());
            REQUIRE_FALSE(varEmpty.isType<int16_t>());
            REQUIRE_FALSE(varEmpty.isType<uint16_t>());
            REQUIRE_FALSE(varEmpty.isType<int32_t>());
            REQUIRE_FALSE(varEmpty.isType<uint32_t>());
            REQUIRE_FALSE(varEmpty.isType<int64_t>());
            REQUIRE_FALSE(varEmpty.isType<uint64_t>());
            REQUIRE_FALSE(varEmpty.isType<float>());
            REQUIRE_FALSE(varEmpty.isType<double>());
        }
    }

    SECTION("Constructors for different types") {
        {
            Variant var(static_cast<bool>(true));
            REQUIRE(var.isScalar());
            REQUIRE(var.isType<bool>());
        }
        {
            Variant var(static_cast<int>(5));
            REQUIRE(var.isScalar());
            REQUIRE(var.isType<int>());
        }
        {
            Variant var(static_cast<double>(11.11));
            REQUIRE(var.isScalar());
            REQUIRE(var.isType<double>());
        }
        // {
        //     Variant var(std::vector<float>({1, 2, 3}));
        //     REQUIRE(var.isArray());
        //     REQUIRE(var.isType<float>());
        // }
    }

    SECTION("Read / write scalar") {
        Variant var;
        int32_t value = 5;
        var.writeScalar(value);

        REQUIRE(var.isScalar());
        REQUIRE(var.isType(Type::Int32));
        REQUIRE(var.isType<int32_t>());

        REQUIRE(var.readScalar<int32_t>());
        REQUIRE_FALSE(var.readArray<int32_t>());
        REQUIRE_FALSE(var.readScalar<bool>());
        
        REQUIRE(var.readScalar<int32_t>().value() == value);
    }

    SECTION("Read / write mixed scalar types") {
        Variant var;

        var.writeScalar(static_cast<int>(11));
        REQUIRE(var.readScalar<int>() == 11);

        var.writeScalar(static_cast<float>(11.11));
        REQUIRE(var.readScalar<float>() == 11.11f);

        var.writeScalar(static_cast<short>(1));
        REQUIRE(var.readScalar<short>() == 1);
    }

    SECTION("Read / write array") {
        Variant var;
        std::vector<float> value({0, 1, 2, 3, 4, 5});
        var.writeArray(value);

        REQUIRE(var.isArray());
        REQUIRE(var.isType(Type::Float));
        REQUIRE(var.isType<float>());

        REQUIRE(var.readArray<float>());
        REQUIRE_FALSE(var.readArray<int32_t>());
        REQUIRE_FALSE(var.readArray<bool>());
        
        REQUIRE(var.readArray<float>().value() == value);
    }

    SECTION("Copy / assignment") {
        int value = 43;
        Variant var(value);
        REQUIRE(var.readScalar<int>() == value);

        Variant varCopy(var);
        REQUIRE(varCopy.readScalar<int>() == value);
        REQUIRE(var.handle()->data != varCopy.handle()->data); // require deep copy

        Variant varCopyAssignment = var;
        REQUIRE(varCopyAssignment.readScalar<int>() == value);
        REQUIRE(var.handle()->data != varCopyAssignment.handle()->data); // require deep copy

        SECTION("Write new value to original variant") {
            float newValue = 11.11f;
            var.writeScalar(newValue);
            REQUIRE(var.readScalar<float>()             == newValue);
            REQUIRE(varCopy.readScalar<int>()           == value);
            REQUIRE(varCopyAssignment.readScalar<int>() == value);
        }
    }

    // SECTION("Operator overloading for easy reading") {
    //     Variant var(static_cast<double>(11.11));

    //     double value = var;
    //     REQUIRE(value == 11.11);
    // }
}