#pragma once

#include <string_view>
#include <type_traits>

#include "open62541pp/detail/types_handling.hpp"  // copyArray
#include "open62541pp/span.hpp"
#include "open62541pp/detail/string_utils.hpp"  // allocNativeString
#include "open62541pp/typeregistry.hpp"  // getDataType
#include "open62541pp/wrapper.hpp"  // asNative, asWrapper, IsWrapper

namespace opcua::detail {

[[nodiscard]] inline auto makeNative(std::string_view value) {
    return detail::allocNativeString(value);
}

template <typename T>
[[nodiscard]] auto makeNative(T value) noexcept
    -> std::enable_if_t<std::is_enum_v<T>, std::underlying_type_t<T>> {
    return static_cast<std::underlying_type_t<T>>(value);
}

template <typename T, typename U = std::remove_cv_t<std::remove_reference_t<T>>>
[[nodiscard]] auto makeNative(T&& value)
    -> std::enable_if_t<IsWrapper<U>::value, typename U::NativeType> {
    using NativeType = typename U::NativeType;
    NativeType native{};
    asWrapper<U>(native) = std::forward<T>(value);
    return native;
}

template <typename T>
[[nodiscard]] auto* makeNativeArray(Span<const T> array) {
    if constexpr (IsWrapper<T>::value) {
        return copyArray(asNative(array.data()), array.size(), getDataType<T>());
    } else {
        return copyArray(array.data(), array.size(), getDataType<T>());
    }
}

}  // namespace opcua::detail
