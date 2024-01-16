#include <cstdint>
#include <limits>

#include <doctest/doctest.h>

#include "open62541pp/BitMask.h"

using namespace opcua;

namespace testenv {
using Underlying = uint8_t;

enum class Access : Underlying {
    Read = 1,
    Write = 2,
};
}  // namespace testenv

namespace opcua {
template <>
struct IsBitMaskEnum<testenv::Access> : std::true_type {};
}  // namespace opcua

TEST_CASE("Bitwise operations with enum (enabled with IsBitMaskEnum trait)") {
    using namespace testenv;

    CHECK(IsBitMaskEnum<Access>::value == true);

    SUBCASE("AND") {
        CHECK(static_cast<Underlying>(Access::Read & Access::Read) == 1);
        CHECK(static_cast<Underlying>(Access::Read & Access::Write) == 0);
    }

    SUBCASE("OR") {
        CHECK(static_cast<Underlying>(Access::Read | Access::Read) == 1);
        CHECK(static_cast<Underlying>(Access::Read | Access::Write) == 3);
    }

    SUBCASE("XOR") {
        CHECK(static_cast<Underlying>(Access::Read ^ Access::Read) == 0);
        CHECK(static_cast<Underlying>(Access::Read ^ Access::Write) == 3);
    }

    SUBCASE("NOT") {
        CHECK(static_cast<Underlying>(~Access::Read) == std::numeric_limits<Underlying>::max() - 1);
    }

    SUBCASE("AND assignment") {
        Access mask{};
        mask = Access::Read;
        mask &= Access::Read;
        CHECK(static_cast<Underlying>(mask) == 1);

        mask = Access::Read;
        mask &= Access::Write;
        CHECK(static_cast<Underlying>(mask) == 0);
    }

    SUBCASE("OR assignment") {
        Access mask{};
        mask = Access::Read;
        mask |= Access::Read;
        CHECK(static_cast<Underlying>(mask) == 1);

        mask = Access::Read;
        mask |= Access::Write;
        CHECK(static_cast<Underlying>(mask) == 3);
    }

    SUBCASE("XOR assignment") {
        Access mask{};
        mask = Access::Read;
        mask ^= Access::Read;
        CHECK(static_cast<Underlying>(mask) == 0);

        mask = Access::Read;
        mask ^= Access::Write;
        CHECK(static_cast<Underlying>(mask) == 3);
    }
}

TEST_CASE("BitMask") {
    using namespace testenv;

    SUBCASE("Constructors") {
        CHECK_NOTHROW(BitMask<Access>());
        CHECK_NOTHROW(BitMask<Access>(Access::Read));
        CHECK_NOTHROW(BitMask<Access>(2));
    }

    SUBCASE("Conversion to enum") {
        CHECK(static_cast<Access>(BitMask<Access>(Access::Read)) == Access::Read);
    }

    SUBCASE("Conversion to underlying") {
        // CHECK(static_cast<Underlying>(BitMask<Access>(2)) == 2);  // deprecated
    }

    SUBCASE("get()") {
        CHECK(BitMask<Access>().get() == 0);
        CHECK(BitMask<Access>(Access::Read).get() == 1);
        CHECK(BitMask<Access>(Access::Write).get() == 2);
    }
}
