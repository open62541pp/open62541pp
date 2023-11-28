#include <doctest/doctest.h>

#include "open62541pp/TypeConverter.h"

using namespace opcua;

TEST_CASE_TEMPLATE("TypeConverter string", T, std::string, std::string_view) {
    SUBCASE("fromNative") {
        const String src("Test123");
        T dst;

        TypeConverter<T>::fromNative(src, dst);
        CHECK(std::string(dst) == "Test123");
    }

    SUBCASE("toNative") {
        T src("Test123");
        String dst;

        TypeConverter<T>::toNative(src, dst);
        CHECK(detail::toString(dst) == "Test123");
    }
}

TEST_CASE("TypeConverter const char*") {
    SUBCASE("toNative") {
        const char* src = "Test123";
        String dst;

        TypeConverter<const char*>::toNative(src, dst);
        CHECK(detail::toString(dst) == "Test123");
    }
}

TEST_CASE("TypeConverter char[N]") {
    SUBCASE("toNative") {
        char src[7] = {'T', 'e', 's', 't', '1', '2', '3'};
        String dst;

        TypeConverter<char[7]>::toNative(src, dst);
        CHECK(detail::toString(dst) == "Test123");
    }
}

TEST_CASE("TypeConverter std::chrono::time_point") {
    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

    SUBCASE("fromNative") {
        const TimePoint src{};  // = Unix epoch
        DateTime dst;

        TypeConverter<TimePoint>::toNative(src, dst);
        CHECK(dst.get() == UA_DATETIME_UNIX_EPOCH);
    }

    SUBCASE("fromNative") {
        const DateTime src = UA_DATETIME_UNIX_EPOCH;
        TimePoint dst = std::chrono::system_clock::now();

        TypeConverter<TimePoint>::fromNative(src, dst);
        CHECK(dst.time_since_epoch().count() == 0);
    }
}
