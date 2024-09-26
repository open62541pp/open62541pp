#pragma once

#include <type_traits>

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

}  // namespace opcua
