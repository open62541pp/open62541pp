#pragma once

#include <string_view>
#include <type_traits>

#include "open62541pp/Span.h"
#include "open62541pp/TypeRegistry.h"  // getDataType
#include "open62541pp/Wrapper.h"  // asNative, asWrapper
#include "open62541pp/detail/string_utils.h"  // allocNativeString
#include "open62541pp/detail/types_handling.h"  // copyArray

namespace opcua::detail {

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
[[nodiscard]] inline auto toNative(T value) noexcept {
    return static_cast<std::underlying_type_t<T>>(value);
}

[[nodiscard]] inline auto toNative(std::string_view value) {
    return allocNativeString(value);
}

template <typename T, typename = std::enable_if_t<isWrapper<T>>>
[[nodiscard]] inline auto toNative(T&& value) {
    using NativeType = typename T::NativeType;
    NativeType native{};
    asWrapper<T>(native) = std::forward<T>(value);
    return native;
}

template <typename T>
[[nodiscard]] inline auto* toNativeArray(Span<const T> array) {
    if constexpr (isWrapper<T>) {
        return copyArray(asNative(array.data()), array.size(), getDataType<T>());
    } else {
        return copyArray(array.data(), array.size(), getDataType<T>());
    }
}

}  // namespace opcua::detail
