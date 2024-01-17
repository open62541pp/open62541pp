#include <cstdint>
#include <limits>

#include <doctest/doctest.h>

#include "open62541pp/BitMask.h"

using namespace opcua;

namespace {
enum class Bit : uint16_t {
    One = 1,
    Two = 2,
};
}

namespace opcua {
template <>
struct IsBitMaskEnum<Bit> : std::true_type {};
}  // namespace opcua

TEST_CASE("Bitwise operations with enum (enabled with IsBitMaskEnum trait)") {
    CHECK(IsBitMaskEnum<Bit>::value == true);

    SUBCASE("AND") {
        CHECK(static_cast<uint16_t>(Bit::One & Bit::One) == 1);
        CHECK(static_cast<uint16_t>(Bit::One & Bit::Two) == 0);
    }

    SUBCASE("OR") {
        CHECK(static_cast<uint16_t>(Bit::One | Bit::One) == 1);
        CHECK(static_cast<uint16_t>(Bit::One | Bit::Two) == 3);
    }

    SUBCASE("XOR") {
        CHECK(static_cast<uint16_t>(Bit::One ^ Bit::One) == 0);
        CHECK(static_cast<uint16_t>(Bit::One ^ Bit::Two) == 3);
    }

    SUBCASE("NOT") {
        CHECK(static_cast<uint16_t>(~Bit::One) == std::numeric_limits<uint16_t>::max() - 1);
    }

    SUBCASE("AND assignment") {
        Bit mask{};
        mask = Bit::One;
        mask &= Bit::One;
        CHECK(static_cast<uint16_t>(mask) == 1);

        mask = Bit::One;
        mask &= Bit::Two;
        CHECK(static_cast<uint16_t>(mask) == 0);
    }

    SUBCASE("OR assignment") {
        Bit mask{};
        mask = Bit::One;
        mask |= Bit::One;
        CHECK(static_cast<uint16_t>(mask) == 1);

        mask = Bit::One;
        mask |= Bit::Two;
        CHECK(static_cast<uint16_t>(mask) == 3);
    }

    SUBCASE("XOR assignment") {
        Bit mask{};
        mask = Bit::One;
        mask ^= Bit::One;
        CHECK(static_cast<uint16_t>(mask) == 0);

        mask = Bit::One;
        mask ^= Bit::Two;
        CHECK(static_cast<uint16_t>(mask) == 3);
    }
}

TEST_CASE("BitMask") {
    SUBCASE("Conversion to enum") {
        CHECK(static_cast<Bit>(BitMask(Bit::One)) == Bit::One);
    }

    SUBCASE("Conversion to uint16_t") {
        // deprecated, enable when deprecation is removed and conversion marked explicit
        // CHECK(static_cast<uint16_t>(BitMask<Bit>(2)) == 2);
    }

    SUBCASE("get()") {
        CHECK(BitMask<Bit>().get() == 0);
        CHECK(BitMask<Bit>(Bit::One).get() == 1);
        CHECK(BitMask<Bit>(2).get() == 2);
    }

    SUBCASE("Equality") {
        CHECK(BitMask<Bit>() == BitMask<Bit>());
        CHECK(BitMask(Bit::One) == BitMask(Bit::One));
        CHECK(BitMask(Bit::One) != BitMask(Bit::Two));

        CHECK(BitMask(Bit::One) == Bit::One);
        CHECK(BitMask(Bit::One) == 1);
        CHECK(BitMask(Bit::One) != Bit::Two);
        CHECK(BitMask(Bit::One) != 2);

        CHECK(Bit::One == BitMask(Bit::One));
        CHECK(1 == BitMask(Bit::One));
        CHECK(Bit::Two != BitMask(Bit::One));
        CHECK(2 != BitMask(Bit::One));
    }
}
