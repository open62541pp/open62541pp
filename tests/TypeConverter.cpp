#include <doctest/doctest.h>

#include "open62541pp/TypeConverter.h"
#include "open62541pp/detail/traits.h"

using namespace opcua;

TEST_CASE("TypeConverter checks") {
    SUBCASE("isValidTypeCombination") {
        using ReadRequest = TypeWrapper<UA_ReadRequest, UA_TYPES_READREQUEST>;
        CHECK(detail::isValidTypeCombination<ReadRequest>(&UA_TYPES[UA_TYPES_READREQUEST]));
        CHECK_FALSE(detail::isValidTypeCombination<ReadRequest>(&UA_TYPES[UA_TYPES_WRITEREQUEST]));
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

TEST_CASE("TypeConverter native types (generated)") {
    CHECK(TypeConverter<UA_NodeClass>::ValidTypes::size() == 1);
    CHECK(TypeConverter<UA_NodeClass>::ValidTypes::contains(UA_TYPES_NODECLASS));
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

TEST_CASE_TEMPLATE("TypeConverter string", T, std::string, std::string_view) {
    SUBCASE("fromNative") {
        UA_String src = detail::allocUaString("Test123");
        T dst;

        TypeConverter<T>::fromNative(src, dst);
        CHECK(std::string(dst) == "Test123");

        UA_clear(&src, &UA_TYPES[UA_TYPES_STRING]);
    }

    SUBCASE("toNative") {
        T src{"Test123"};
        UA_String dst{};

        TypeConverter<T>::toNative(src, dst);
        CHECK(detail::toString(dst) == "Test123");

        UA_clear(&dst, &UA_TYPES[UA_TYPES_STRING]);
    }
}

TEST_CASE("TypeConverter const char*") {
    SUBCASE("toNative") {
        const char* src = "Test123";
        UA_String dst{};

        TypeConverter<const char*>::toNative(src, dst);
        CHECK(detail::toString(dst) == "Test123");

        UA_clear(&dst, &UA_TYPES[UA_TYPES_STRING]);
    }
}

TEST_CASE("TypeConverter char[N]") {
    SUBCASE("toNative") {
        char src[7] = {'T', 'e', 's', 't', '1', '2', '3'};
        UA_String dst{};

        TypeConverter<char[7]>::toNative(src, dst);
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
