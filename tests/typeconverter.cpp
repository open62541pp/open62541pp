#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

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
    SECTION("fromNative") {
        UA_Byte src{101};
        Byte dst{};
        TypeConverter<Byte>::fromNative(src, dst);
        CHECK(static_cast<int>(dst) == 101);
    }

    SECTION("toNative") {
        Byte src{101};
        UA_Byte dst{};
        TypeConverter<Byte>::toNative(src, dst);
        CHECK(static_cast<int>(dst) == 101);
    }
}

TEST_CASE("TypeConverter helper functions") {
    SECTION("fromNative") {
        UA_Byte src{101};
        Byte dst = detail::fromNative<Byte>(src);
        CHECK(static_cast<int>(dst) == 101);
    }

    SECTION("toNative") {
        Byte src{101};
        UA_Byte dst = detail::toNative(src);
        CHECK(static_cast<int>(dst) == 101);
    }
}

TEMPLATE_TEST_CASE("TypeConverter string", "", std::string, std::string_view) {
    SECTION("fromNative") {
        const String src("Test123");
        TestType dst = detail::fromNative<TestType>(src);
        CHECK(std::string(dst) == "Test123");
    }

    SECTION("toNative") {
        TestType src("Test123");
        String dst = detail::toNative(src);
        CHECK(dst == "Test123");
    }
}

TEST_CASE("TypeConverter const char*") {
    SECTION("toNative") {
        const char* src = "Test123";
        String dst = detail::toNative(src);
        CHECK(dst == "Test123");
    }
}

TEST_CASE("TypeConverter char[N]") {
    SECTION("toNative") {
        char src[7] = {'T', 'e', 's', 't', '1', '2', '3'};
        String dst = detail::toNative(src);
        CHECK(dst == "Test123");
    }
}

TEST_CASE("TypeConverter std::chrono::time_point") {
    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

    SECTION("fromNative") {
        const DateTime src = UA_DATETIME_UNIX_EPOCH;
        TimePoint dst = std::chrono::system_clock::now();
        dst = detail::fromNative<TimePoint>(src);
        CHECK(dst.time_since_epoch().count() == 0);
    }

    SECTION("toNative") {
        const TimePoint src{};  // = Unix epoch
        DateTime dst = detail::toNative(src);
        CHECK(dst.get() == UA_DATETIME_UNIX_EPOCH);
    }
}
