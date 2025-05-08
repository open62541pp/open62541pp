#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "open62541pp/typeconverter.hpp"
#include "open62541pp/types.hpp"

using namespace opcua;

enum class ByteVal : uint8_t {};
enum class ByteRef : uint8_t {};

namespace opcua {
template <>
// 1. option: return converted value
struct TypeConverter<ByteVal> {
    using NativeType = UA_Byte;

    [[nodiscard]] static ByteVal fromNative(const UA_Byte& src) {
        return static_cast<ByteVal>(src);
    }

    [[nodiscard]] static UA_Byte toNative(const ByteVal& src) {
        return static_cast<uint8_t>(src);
    }
};

// 2. option: take output parameter by reference
template <>
struct TypeConverter<ByteRef> {
    using NativeType = UA_Byte;

    static void fromNative(const UA_Byte& src, ByteRef& dst) {
        dst = static_cast<ByteRef>(src);
    }

    static void toNative(const ByteRef& src, UA_Byte& dst) {
        dst = static_cast<uint8_t>(src);
    }
};
}  // namespace opcua

TEST_CASE("TypeConverter helper functions fromNative/toNative") {
    SECTION("fromNative by return value") {
        const UA_Byte src{101};
        const ByteVal dst = detail::fromNative<ByteVal>(src);
        CHECK(static_cast<int>(dst) == 101);
    }

    SECTION("fromNative by reference parameter") {
        const UA_Byte src{101};
        const ByteRef dst = detail::fromNative<ByteRef>(src);
        CHECK(static_cast<int>(dst) == 101);
    }

    SECTION("toNative by return value") {
        const ByteVal src{101};
        const UA_Byte dst = detail::toNative(src);
        CHECK(static_cast<int>(dst) == 101);
    }

    SECTION("toNative by reference parameter") {
        const ByteRef src{101};
        const UA_Byte dst = detail::toNative(src);
        CHECK(static_cast<int>(dst) == 101);
    }
}

TEMPLATE_TEST_CASE("TypeConverter string", "", std::string, std::string_view) {
    SECTION("fromNative") {
        const String src{"Test123"};
        const TestType dst = detail::fromNative<TestType>(src);
        CHECK(std::string{dst} == "Test123");
    }

    SECTION("toNative") {
        const TestType src{"Test123"};
        const String dst = detail::toNative(src);
        CHECK(dst == "Test123");
    }
}

TEST_CASE("TypeConverter const char*") {
    SECTION("toNative") {
        const char* src = "Test123";
        const String dst = detail::toNative(src);
        CHECK(dst == "Test123");
    }
}

TEST_CASE("TypeConverter char[N]") {
    SECTION("toNative") {
        SECTION("Runtime buffer") {
            char src[7] = {'T', 'e', 's', 't', '1', '2', '3'};
            const String dst = detail::toNative(src);
            CHECK(dst == "Test123");
        }
        SECTION("String literal") {
            const String dst = detail::toNative("Test123");
            CHECK(dst == "Test123");
        }
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
        const DateTime dst = detail::toNative(src);
        CHECK(dst.get() == UA_DATETIME_UNIX_EPOCH);
    }
}
