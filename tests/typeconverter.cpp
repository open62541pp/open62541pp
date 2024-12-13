#include <doctest/doctest.h>

#include "open62541pp/typeconverter.hpp"
#include "open62541pp/types.hpp"

using namespace opcua;

enum class Byte : uint8_t {};

namespace opcua {
template <>
struct TypeConverter<Byte> {
    using NativeType = UA_Byte;

    static void fromNative(const UA_Byte& src, Byte& dst) {
        dst = static_cast<Byte>(src);
    }

    static void toNative(const Byte& src, UA_Byte& dst) {
        dst = static_cast<uint8_t>(src);
    }
};
}  // namespace opcua

TEST_CASE("TypeConverter") {
    SUBCASE("fromNative") {
        UA_Byte src{101};
        Byte dst{};
        TypeConverter<Byte>::fromNative(src, dst);
        CHECK(static_cast<int>(dst) == 101);
    }

    SUBCASE("toNative") {
        Byte src{101};
        UA_Byte dst{};
        TypeConverter<Byte>::toNative(src, dst);
        CHECK(static_cast<int>(dst) == 101);
    }
}

TEST_CASE("TypeConverter helper functions") {
    SUBCASE("fromNative") {
        UA_Byte src{101};
        Byte dst = detail::fromNative<Byte>(src);
        CHECK(static_cast<int>(dst) == 101);
    }

    SUBCASE("toNative") {
        Byte src{101};
        UA_Byte dst = detail::toNative(src);
        CHECK(static_cast<int>(dst) == 101);
    }
}

TEST_CASE_TEMPLATE("TypeConverter string", T, std::string, std::string_view) {
    SUBCASE("fromNative") {
        const String src("Test123");
        T dst = detail::fromNative<T>(src);
        CHECK(std::string(dst) == "Test123");
    }

    SUBCASE("toNative") {
        T src("Test123");
        String dst = detail::toNative(src);
        CHECK(detail::toString(dst) == "Test123");
    }
}

TEST_CASE("TypeConverter const char*") {
    SUBCASE("toNative") {
        const char* src = "Test123";
        String dst = detail::toNative(src);
        CHECK(detail::toString(dst) == "Test123");
    }
}

TEST_CASE("TypeConverter char[N]") {
    SUBCASE("toNative") {
        char src[7] = {'T', 'e', 's', 't', '1', '2', '3'};
        String dst = detail::toNative(src);
        CHECK(detail::toString(dst) == "Test123");
    }
}

TEST_CASE("TypeConverter std::chrono::time_point") {
    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

    SUBCASE("fromNative") {
        const DateTime src = UA_DATETIME_UNIX_EPOCH;
        TimePoint dst = std::chrono::system_clock::now();
        dst = detail::fromNative<TimePoint>(src);
        CHECK(dst.time_since_epoch().count() == 0);
    }

    SUBCASE("toNative") {
        const TimePoint src{};  // = Unix epoch
        DateTime dst = detail::toNative(src);
        CHECK(dst.get() == UA_DATETIME_UNIX_EPOCH);
    }
}
