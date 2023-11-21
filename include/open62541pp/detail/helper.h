#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <tuple>

#include "open62541pp/Common.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/detail/traits.h"
#include "open62541pp/open62541.h"

namespace opcua::detail {

/* ------------------------------------ Generic type handling ----------------------------------- */

using PointerFreeTypes = std::tuple<
    UA_Boolean,
    UA_SByte,
    UA_Byte,
    UA_Int16,
    UA_UInt16,
    UA_Int32,
    UA_UInt32,
    UA_Int64,
    UA_UInt64,
    UA_Float,
    UA_Double,
    UA_DateTime,
    UA_Guid,
    UA_StatusCode>;

template <typename T>
inline constexpr bool isPointerFree = TupleHolds<PointerFreeTypes, T>::value;

template <typename T>
constexpr void clear(T& native, const UA_DataType& type) noexcept {
    assert(sizeof(T) == type.memSize);
    if constexpr (!isPointerFree<T>) {
        UA_clear(&native, &type);
    }
}

template <typename T>
[[nodiscard]] constexpr T copy(const T& src, const UA_DataType& type) noexcept(isPointerFree<T>) {
    assert(sizeof(T) == type.memSize);
    if constexpr (!isPointerFree<T>) {
        T dst;  // NOLINT, initialized in UA_copy function
        detail::throwOnBadStatus(UA_copy(&src, &dst, &type));
        return dst;
    } else {
        return src;
    }
}

/* ------------------------------------------ Data type ----------------------------------------- */

/// Get UA_DataType by type index or enum (template parameter).
template <auto typeIndexOrEnum>
inline const UA_DataType& getUaDataType() noexcept {
    constexpr auto typeIndex = static_cast<TypeIndex>(typeIndexOrEnum);
    static_assert(typeIndex < UA_TYPES_COUNT);
    return UA_TYPES[typeIndex];  // NOLINT
}

/// Get UA_DataType by type index.
inline const UA_DataType& getUaDataType(TypeIndex typeIndex) noexcept {
    assert(typeIndex < UA_TYPES_COUNT);
    return UA_TYPES[typeIndex];  // NOLINT
}

/// Get UA_DataType by type enum.
inline const UA_DataType& getUaDataType(Type type) noexcept {
    return getUaDataType(static_cast<TypeIndex>(type));
}

/* ---------------------------------------- String utils ---------------------------------------- */

/// Convert std::string_view to UA_String (no copy)
UA_String toUaString(std::string_view src) noexcept;

/// Allocate UA_String from std::string_view
[[nodiscard]] UA_String allocUaString(std::string_view src);

/// Check if UA_String is empty
constexpr bool isEmpty(const UA_String& value) {
    return (value.data == nullptr) || (value.length == 0);
}

/// Convert UA_String to std::string_view
inline std::string_view toStringView(const UA_String& src) {
    if (isEmpty(src)) {
        return {};
    }
    return {(char*)src.data, src.length};  // NOLINT
}

/// Convert UA_String to std::string
inline std::string toString(const UA_String& src) {
    return std::string(toStringView(src));
}

}  // namespace opcua::detail
