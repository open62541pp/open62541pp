#pragma once

#include <array>
#include <chrono>
#include <string>
#include <string_view>
#include <type_traits>

#include "open62541pp/Common.h"  // TypeIndex
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/DateTime.h"

namespace opcua {

/**
 * Type conversion from and to native types.
 *
 * Native types can be both `UA_*` types and wrapper classes (like `UA_Guid` and `Guid`).
 * The `TypeConverter` is mainly used within the `Variant` class to set/get non-native types.
 *
 * Template specializations can be added for conversions of arbitrary types:
 * @code
 * namespace ::opcua {
 * template <>
 * struct TypeConverter<MyCustomType> {
 *     using NativeType = Guid;
 *
 *     static void fromNative(const NativeType& src, MyCustomType& dst) {
 *         // ...
 *     }
 *
 *     static void toNative(const MyCustomType& src, NativeType& dst) {
 *         // ...
 *     }
 * };
 * }
 * @endcode
 */
template <typename T, typename Enable = void>
struct TypeConverter;

/* -------------------------------------- Traits and helper ------------------------------------- */

namespace detail {

template <typename T, typename = void>
struct IsConvertibleType : std::false_type {};

template <typename T>
struct IsConvertibleType<T, std::void_t<decltype(TypeConverter<T>{})>> : std::true_type {};

template <typename T>
inline constexpr bool isConvertibleType = IsConvertibleType<T>::value;

}  // namespace detail

/* ---------------------------------- Template specializations ---------------------------------- */

template <>
struct TypeConverter<std::string_view> {
    using ValueType = std::string_view;
    using NativeType = String;

    static void fromNative(const NativeType& src, std::string_view& dst) {
        dst = static_cast<std::string_view>(src);
    }

    static void toNative(std::string_view src, NativeType& dst) {
        dst = String(src);
    }
};

template <>
struct TypeConverter<std::string> {
    using ValueType = std::string;
    using NativeType = String;

    static void fromNative(const NativeType& src, ValueType& dst) {
        dst = std::string(src);
    }

    static void toNative(const ValueType& src, NativeType& dst) {
        dst = String(src);
    }
};

template <>
struct TypeConverter<const char*> {
    using ValueType = const char*;
    using NativeType = String;

    static void toNative(const char* src, NativeType& dst) {
        dst = String(src);
    }
};

template <size_t N>
struct TypeConverter<char[N]> {  // NOLINT
    using ValueType = char[N];  // NOLINT
    using NativeType = String;

    static void toNative(const ValueType& src, NativeType& dst) {
        dst = String({static_cast<const char*>(src), N});
    }
};

template <typename Clock, typename Duration>
struct TypeConverter<std::chrono::time_point<Clock, Duration>> {
    using ValueType = std::chrono::time_point<Clock, Duration>;
    using NativeType = DateTime;

    static void fromNative(const NativeType& src, ValueType& dst) {
        dst = src.toTimePoint<Clock, Duration>();
    }

    static void toNative(const ValueType& src, NativeType& dst) {
        dst = DateTime::fromTimePoint(src);
    }
};

}  // namespace opcua
