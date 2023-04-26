#include <cstdint>

#include <doctest/doctest.h>

#include "open62541pp/Mask.h"

using namespace opcua;

enum class Flags : uint8_t {
    Bit1 = 0b00000001,
    Bit2 = 0b00000010,
    Bit3 = 0b00000100,
    Bit4 = 0b00001000,
    Bit5 = 0b00010000,
    Bit6 = 0b00100000,
    Bit7 = 0b01000000,
    Bit8 = 0b10000000,
};

TEST_CASE("Mask") {
    SUBCASE("Empty") {
        Mask<Flags> mask;
        CHECK(mask.get() == 0b00000000);
        CHECK(mask.none());
        CHECK_FALSE(mask.any());
        CHECK_FALSE(mask.all());
    }

    SUBCASE("Construct with enum value") {
        Mask<Flags> mask = Flags::Bit3;
        CHECK(mask.get() == 0b00000100);
    }

    SUBCASE("Construct with enum values") {
        Mask<Flags> mask{Flags::Bit1, Flags::Bit3};
        CHECK(mask.get() == 0b00000101);
    }

    SUBCASE("Construct with underlying value") {
        Mask<Flags> mask = 1 | 2;
        CHECK(mask.get() == 0b00000011);
    }

    SUBCASE("Set flag") {
        Mask<Flags> mask;
        mask.set(Flags::Bit7);
        mask.set(Flags::Bit8);
        CHECK(mask.get() == 0b11000000);
    }

    SUBCASE("Reset flags") {
        Mask<Flags> mask{Flags::Bit1, Flags::Bit2};
        mask.reset();
        CHECK(mask.get() == 0b00000000);
    }

    SUBCASE("Reset specific flag") {
        Mask<Flags> mask = Flags::Bit2;
        mask.reset(Flags::Bit2);
        CHECK(mask.get() == 0b00000000);
    }

    SUBCASE("Flip bits") {
        Mask<Flags> mask = Flags::Bit2;
        mask.flip();
        CHECK(mask.get() == 0b11111101);
    }

    SUBCASE("Test flags") {
        Mask<Flags> mask{Flags::Bit1, Flags::Bit2};
        CHECK(mask.test(Flags::Bit1));
        CHECK(mask.test(Flags::Bit2));
        CHECK_FALSE(mask.test(Flags::Bit3));
    }

    SUBCASE("None") {
        CHECK(Mask<Flags>().none());
        CHECK_FALSE(Mask<Flags>(Flags::Bit1).none());
    }

    SUBCASE("Any") {
        CHECK_FALSE(Mask<Flags>().any());
        CHECK(Mask<Flags>(Flags::Bit1).any());
    }

    SUBCASE("All") {
        CHECK_FALSE(Mask<Flags>(Flags::Bit1).all());
        CHECK(Mask<Flags>(0b11111111).all());
    }
}

TEST_CASE("Mask overloads") {
    SUBCASE("OR") {
        CHECK((Mask<Flags>(Flags::Bit1) | Mask<Flags>(Flags::Bit2)).get() == 0b00000011);
        CHECK((Mask<Flags>(Flags::Bit1) | Flags::Bit2).get() == 0b00000011);
        CHECK((Flags::Bit1 | Mask<Flags>(Flags::Bit2)).get() == 0b00000011);
    }

    SUBCASE("OR assignment") {
        Mask<Flags> mask;
        mask |= Flags::Bit1;
        CHECK(mask.get() == 0b00000001);
        mask |= Mask<Flags>(Flags::Bit2);
        CHECK(mask.get() == 0b00000011);
    }

    SUBCASE("Comparison") {
        CHECK(Mask<Flags>(Flags::Bit1) == 0b00000001);
        CHECK(Mask<Flags>(Flags::Bit1) != 0);

        CHECK(1 == Mask<Flags>(Flags::Bit1));
        CHECK(1 != Mask<Flags>());
    }
}
