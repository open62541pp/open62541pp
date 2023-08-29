#pragma once

#include <cassert>
#include <string>
#include <string_view>

#include "open62541pp/Common.h"
#include "open62541pp/open62541.h"

namespace opcua::detail {

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

/// Find (custom) UA_DataType by UA_NodeId.
/// Return nullptr if no matching data type was found.
inline const UA_DataType* findUaDataType(const UA_NodeId& id) noexcept {
    return UA_findDataType(&id);
}

/// Allocate UA_String from const char*
[[nodiscard]] UA_String allocUaString(const char* src);

/// Allocate UA_String from std::string
[[nodiscard]] UA_String allocUaString(const std::string& src);

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
