#pragma once

#include <string_view>

#include "open62541pp/detail/open62541/common.h"  // UA_String

namespace opcua::detail {

/// Convert std::string_view to UA_String (no copy)
UA_String toNativeString(std::string_view src) noexcept;

/// Allocate UA_String from std::string_view
[[nodiscard]] UA_String allocNativeString(std::string_view src);

/// Allocate const char* from std::string_view
[[nodiscard]] char* allocCString(std::string_view src);

void clear(const char* str) noexcept;

/// Convert UA_String to std::string_view
/// Can be marked noexcept: https://stackoverflow.com/a/62061549/9967707
inline std::string_view toStringView(const UA_String& src) noexcept {
    if (src.data == nullptr || src.length == 0U) {
        return {};
    }
    return {(const char*)src.data, src.length};  // NOLINT
}

}  // namespace opcua::detail
