#include <cstdint>

#include <doctest/doctest.h>

#include "open62541pp/Bitmask.h"

using namespace opcua;

namespace {
enum class Bit : int {
    One = 1,
    Two = 2,
};
}

namespace opcua {
template <>
struct IsBitmaskEnum<Bit> : std::true_type {};
}  // namespace opcua

TEST_CASE("Bitwise operations with enum (enabled with IsBitmaskEnum trait)") {
    CHECK(IsBitmaskEnum<Bit>::value == true);

    SUBCASE("AND") {
        CHECK(static_cast<int>(Bit::One & Bit::One) == 1);
        CHECK(static_cast<int>(Bit::One & Bit::Two) == 0);
    }

    SUBCASE("OR") {
        CHECK(static_cast<int>(Bit::One | Bit::One) == 1);
        CHECK(static_cast<int>(Bit::One | Bit::Two) == 3);
    }

    SUBCASE("XOR") {
        CHECK(static_cast<int>(Bit::One ^ Bit::One) == 0);
        CHECK(static_cast<int>(Bit::One ^ Bit::Two) == 3);
    }

    SUBCASE("NOT") {
        CHECK(static_cast<int>(~Bit::One) == ~1);
    }

    SUBCASE("AND assignment") {
        Bit mask{};
        mask = Bit::One;
        mask &= Bit::One;
        CHECK(static_cast<int>(mask) == 1);

        mask = Bit::One;
        mask &= Bit::Two;
        CHECK(static_cast<int>(mask) == 0);
    }

    SUBCASE("OR assignment") {
        Bit mask{};
        mask = Bit::One;
        mask |= Bit::One;
        CHECK(static_cast<int>(mask) == 1);

        mask = Bit::One;
        mask |= Bit::Two;
        CHECK(static_cast<int>(mask) == 3);
    }

    SUBCASE("XOR assignment") {
        Bit mask{};
        mask = Bit::One;
        mask ^= Bit::One;
        CHECK(static_cast<int>(mask) == 0);

        mask = Bit::One;
        mask ^= Bit::Two;
        CHECK(static_cast<int>(mask) == 3);
    }
}

TEST_CASE("Bitmask") {
    SUBCASE("Conversion to enum") {
        CHECK(static_cast<Bit>(Bitmask(Bit::One)) == Bit::One);
    }

    SUBCASE("Conversion to int") {
        // deprecated, enable when deprecation is removed and conversion marked explicit
        // CHECK(static_cast<int>(Bitmask<Bit>(2)) == 2);
    }

    SUBCASE("get()") {
        CHECK(Bitmask<Bit>().get() == 0);
        CHECK(Bitmask<Bit>(Bit::One).get() == 1);
        CHECK(Bitmask<Bit>(2).get() == 2);
    }

    SUBCASE("Equality") {
        CHECK(Bitmask<Bit>() == Bitmask<Bit>());
        CHECK(Bitmask(Bit::One) == Bitmask(Bit::One));
        CHECK(Bitmask(Bit::One) != Bitmask(Bit::Two));

        CHECK(Bitmask(Bit::One) == Bit::One);
        CHECK(Bitmask(Bit::One) == 1);
        CHECK(Bitmask(Bit::One) != Bit::Two);
        CHECK(Bitmask(Bit::One) != 2);

        // ambiguous due to implicit conversion to underlying type
        // CHECK(Bit::One == Bitmask(Bit::One));
        // CHECK(1 == Bitmask(Bit::One));
        // CHECK(Bit::Two != Bitmask(Bit::One));
        // CHECK(2 != Bitmask(Bit::One));
    }
}
