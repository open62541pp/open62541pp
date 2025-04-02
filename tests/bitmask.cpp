#include <type_traits>  // true_type

#include <catch2/catch_test_macros.hpp>

#include "open62541pp/bitmask.hpp"

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

TEST_CASE("Enable bitwise operations (IsBitmaskEnum trait)") {
    CHECK(IsBitmaskEnum<Bit>::value == true);
}

namespace nested {
enum class Bit2 : int {
    One = 1,
    Two = 2,
};
constexpr std::true_type isBitmaskEnum(Bit2);
}  // namespace nested

TEST_CASE("Enable bitwise operations (isBitmaskEnum function overload)") {
    CHECK(IsBitmaskEnum<nested::Bit2>::value == true);
}

TEST_CASE("Bitwise operations with enum (enabled with IsBitmaskEnum trait)") {
    SECTION("AND") {
        CHECK(static_cast<int>(Bit::One & Bit::One) == 1);
        CHECK(static_cast<int>(Bit::One & Bit::Two) == 0);
    }

    SECTION("OR") {
        CHECK(static_cast<int>(Bit::One | Bit::One) == 1);
        CHECK(static_cast<int>(Bit::One | Bit::Two) == 3);
    }

    SECTION("XOR") {
        CHECK(static_cast<int>(Bit::One ^ Bit::One) == 0);
        CHECK(static_cast<int>(Bit::One ^ Bit::Two) == 3);
    }

    SECTION("NOT") {
        CHECK(static_cast<int>(~Bit::One) == ~1);
    }

    SECTION("AND assignment") {
        Bit mask{};
        mask = Bit::One;
        mask &= Bit::One;
        CHECK(static_cast<int>(mask) == 1);

        mask = Bit::One;
        mask &= Bit::Two;
        CHECK(static_cast<int>(mask) == 0);
    }

    SECTION("OR assignment") {
        Bit mask{};
        mask = Bit::One;
        mask |= Bit::One;
        CHECK(static_cast<int>(mask) == 1);

        mask = Bit::One;
        mask |= Bit::Two;
        CHECK(static_cast<int>(mask) == 3);
    }

    SECTION("XOR assignment") {
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
    SECTION("Conversion to enum") {
        CHECK(static_cast<Bit>(Bitmask(Bit::One)) == Bit::One);
    }

    SECTION("Conversion to int") {
        CHECK(static_cast<int>(Bitmask<Bit>(2)) == 2);
    }

    SECTION("get") {
        CHECK(Bitmask<Bit>().get() == 0);
        CHECK(Bitmask<Bit>(Bit::One).get() == 1);
        CHECK(Bitmask<Bit>(2).get() == 2);
    }

    SECTION("all") {
        CHECK(Bitmask<Bit>(0x00000000).all() == false);
        CHECK(Bitmask<Bit>(0xF0F0F0F0).all() == false);
        CHECK(Bitmask<Bit>(0xFFFFFFFF).all());
    }

    SECTION("any") {
        CHECK(Bitmask<Bit>(0x00000000).any() == false);
        CHECK(Bitmask<Bit>(0xF0F0F0F0).any());
        CHECK(Bitmask<Bit>(0xFFFFFFFF).any());
    }

    SECTION("none") {
        CHECK(Bitmask<Bit>(0x00000000).none());
        CHECK(Bitmask<Bit>(0xF0F0F0F0).none() == false);
        CHECK(Bitmask<Bit>(0xFFFFFFFF).none() == false);
    }

    SECTION("allOf") {
        CHECK(Bitmask<Bit>(Bit::One).allOf(Bit::One) == true);
        CHECK(Bitmask<Bit>(Bit::One).allOf(Bit::Two) == false);
        CHECK(Bitmask<Bit>(Bit::One).allOf(Bit::One | Bit::Two) == false);
    }

    SECTION("anyOf") {
        CHECK(Bitmask<Bit>(Bit::One).anyOf(Bit::One) == true);
        CHECK(Bitmask<Bit>(Bit::One).anyOf(Bit::Two) == false);
        CHECK(Bitmask<Bit>(Bit::One).anyOf(Bit::One | Bit::Two) == true);
    }

    SECTION("noneOf") {
        CHECK(Bitmask<Bit>(Bit::One).noneOf(Bit::One) == false);
        CHECK(Bitmask<Bit>(Bit::One).noneOf(Bit::Two) == true);
        CHECK(Bitmask<Bit>(Bit::One).noneOf(Bit::One | Bit::Two) == false);
    }

    SECTION("set") {
        CHECK(Bitmask<Bit>().set().get() == static_cast<int>(0xFFFFFFFF));
        CHECK(Bitmask<Bit>().set(Bit::One).get() == 1);
    }

    SECTION("reset") {
        CHECK(Bitmask<Bit>(Bit::One | Bit::Two).reset().get() == 0);
        CHECK(Bitmask<Bit>(Bit::One | Bit::Two).reset(Bit::One).get() == 2);
    }

    SECTION("flip") {
        CHECK(Bitmask<Bit>(0).flip().get() == static_cast<int>(0xFFFFFFFF));
        CHECK(Bitmask<Bit>(1).flip().get() == static_cast<int>(0xFFFFFFFE));
    }

    SECTION("Equality") {
        CHECK(Bitmask<Bit>() == Bitmask<Bit>());

        CHECK(Bitmask(Bit::One) == Bitmask(Bit::One));
        CHECK(Bitmask(Bit::One) == Bit::One);
        CHECK(Bit::One == Bitmask(Bit::One));
        CHECK(Bitmask(Bit::One) == 1);
        CHECK(1 == Bitmask(Bit::One));

        CHECK(Bitmask(Bit::One) != Bitmask(Bit::Two));
        CHECK(Bitmask(Bit::One) != Bit::Two);
        CHECK(Bit::Two != Bitmask(Bit::One));
        CHECK(Bitmask(Bit::One) != 2);
        CHECK(2 != Bitmask(Bit::One));
    }
}
