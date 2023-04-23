#include <doctest/doctest.h>

#include "open62541pp/TypeConverter.h"

using namespace opcua;

TEST_CASE("TypeConverter checks") {
    SUBCASE("isValidTypeCombination") {
        CHECK(detail::isValidTypeCombination<bool>(Type::Boolean));
        CHECK_FALSE(detail::isValidTypeCombination<bool>(Type::Float));

        using ReadRequest = TypeWrapper<UA_ReadRequest, UA_TYPES_READREQUEST>;
        CHECK(detail::isValidTypeCombination<ReadRequest>(UA_TYPES_READREQUEST));
        CHECK_FALSE(detail::isValidTypeCombination<ReadRequest>(UA_TYPES_WRITEREQUEST));
    }
}

TEST_CASE_TEMPLATE(
    "TypeConverter native scalars",
    T,
    UA_SByte,
    UA_Byte,
    UA_Int16,
    UA_UInt16,
    UA_Int32,
    UA_UInt32,
    UA_Int64,
    UA_UInt64,
    UA_Float,
    UA_Double
) {
    const T src{11};

    SUBCASE("fromNative") {
        T dst{};
        TypeConverter<T>::fromNative(src, dst);
        CHECK(dst == src);
    }

    SUBCASE("toNative") {
        T dst{};
        TypeConverter<T>::toNative(src, dst);
        CHECK(dst == src);
    }
}

TEST_CASE("TypeConverter wrapper types") {
    using FloatWrapper = TypeWrapper<float, UA_TYPES_FLOAT>;

    SUBCASE("fromNative") {
        float native = 11.11f;
        FloatWrapper wrapper;
        TypeConverter<FloatWrapper>::fromNative(native, wrapper);
        CHECK(*wrapper.handle() == 11.11f);
    }

    SUBCASE("toNative") {
        FloatWrapper wrapper(11.11f);
        float native{};
        TypeConverter<FloatWrapper>::toNative(wrapper, native);
        CHECK(native == 11.11f);
    }
}

TEST_CASE_TEMPLATE("TypeConverter native strings", T, UA_String, UA_ByteString, UA_XmlElement) {
    T src = detail::allocUaString("Test123");
    T dst = detail::allocUaString("Overwrite me");

    TypeConverter<T>::fromNative(src, dst);
    CHECK(UA_String_equal(&src, &dst));
    CHECK(src.data != dst.data);

    UA_clear(&src, &UA_TYPES[UA_TYPES_STRING]);
    UA_clear(&dst, &UA_TYPES[UA_TYPES_STRING]);
}

TEST_CASE("TypeConverter std::string") {
    SUBCASE("fromNative") {
        UA_String src = detail::allocUaString("Test123");
        std::string dst;

        TypeConverter<std::string>::fromNative(src, dst);
        CHECK(dst == "Test123");

        UA_clear(&src, &UA_TYPES[UA_TYPES_STRING]);
    }

    SUBCASE("toNative") {
        std::string src{"Test123"};
        UA_String dst{};

        TypeConverter<std::string>::toNative(src, dst);
        CHECK(detail::toString(dst) == "Test123");

        UA_clear(&dst, &UA_TYPES[UA_TYPES_STRING]);
    }
}

TEST_CASE("TypeConverter std::chrono::time_point") {
    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

    SUBCASE("fromNative") {
        TimePoint src{};  // = Unix epoch
        UA_DateTime dst{};

        TypeConverter<TimePoint>::toNative(src, dst);
        CHECK(dst == UA_DATETIME_UNIX_EPOCH);
    }

    SUBCASE("fromNative") {
        UA_DateTime src = UA_DATETIME_UNIX_EPOCH;
        TimePoint dst = std::chrono::system_clock::now();

        TypeConverter<TimePoint>::fromNative(src, dst);
        CHECK(dst.time_since_epoch().count() == 0);
    }
}
