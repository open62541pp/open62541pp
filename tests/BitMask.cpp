#include <cstdint>
#include <limits>

#include <doctest/doctest.h>

#include "open62541pp/BitMask.h"

using namespace opcua;

namespace {
enum class BitMaskEnum : uint16_t {
    One = 1,
    Two = 2,
};
}

namespace opcua {
template <>
struct IsBitMaskEnum<BitMaskEnum> : std::true_type {};
}  // namespace opcua

TEST_CASE("Bitwise operations with enum (enabled with IsBitMaskEnum trait)") {
    CHECK(IsBitMaskEnum<BitMaskEnum>::value == true);

    SUBCASE("AND") {
        CHECK(static_cast<uint16_t>(BitMaskEnum::One & BitMaskEnum::One) == 1);
        CHECK(static_cast<uint16_t>(BitMaskEnum::One & BitMaskEnum::Two) == 0);
    }

    SUBCASE("OR") {
        CHECK(static_cast<uint16_t>(BitMaskEnum::One | BitMaskEnum::One) == 1);
        CHECK(static_cast<uint16_t>(BitMaskEnum::One | BitMaskEnum::Two) == 3);
    }

    SUBCASE("XOR") {
        CHECK(static_cast<uint16_t>(BitMaskEnum::One ^ BitMaskEnum::One) == 0);
        CHECK(static_cast<uint16_t>(BitMaskEnum::One ^ BitMaskEnum::Two) == 3);
    }

    SUBCASE("NOT") {
        CHECK(static_cast<uint16_t>(~BitMaskEnum::One) == std::numeric_limits<uint16_t>::max() - 1);
    }

    SUBCASE("AND assignment") {
        BitMaskEnum mask{};
        mask = BitMaskEnum::One;
        mask &= BitMaskEnum::One;
        CHECK(static_cast<uint16_t>(mask) == 1);

        mask = BitMaskEnum::One;
        mask &= BitMaskEnum::Two;
        CHECK(static_cast<uint16_t>(mask) == 0);
    }

    SUBCASE("OR assignment") {
        BitMaskEnum mask{};
        mask = BitMaskEnum::One;
        mask |= BitMaskEnum::One;
        CHECK(static_cast<uint16_t>(mask) == 1);

        mask = BitMaskEnum::One;
        mask |= BitMaskEnum::Two;
        CHECK(static_cast<uint16_t>(mask) == 3);
    }

    SUBCASE("XOR assignment") {
        BitMaskEnum mask{};
        mask = BitMaskEnum::One;
        mask ^= BitMaskEnum::One;
        CHECK(static_cast<uint16_t>(mask) == 0);

        mask = BitMaskEnum::One;
        mask ^= BitMaskEnum::Two;
        CHECK(static_cast<uint16_t>(mask) == 3);
    }
}

TEST_CASE("BitMask") {
    SUBCASE("Conversion to enum") {
        CHECK(static_cast<BitMaskEnum>(BitMask<BitMaskEnum>(BitMaskEnum::One)) == BitMaskEnum::One);
    }

    SUBCASE("Conversion to uint16_t") {
        // deprecated, enable when deprecation is removed and conversion marked explicit
        // CHECK(static_cast<uint16_t>(BitMask<BitMaskEnum>(2)) == 2);
    }

    SUBCASE("get()") {
        CHECK(BitMask<BitMaskEnum>().get() == 0);
        CHECK(BitMask<BitMaskEnum>(BitMaskEnum::One).get() == 1);
        CHECK(BitMask<BitMaskEnum>(2).get() == 2);
    }
}
