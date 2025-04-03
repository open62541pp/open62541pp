#pragma once

#include <cstdarg>  // va_list
#include <string>
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

/// Convert UA_String to std::string
inline std::string toString(const UA_String& src) {
    return std::string(toStringView(src));
}

/// Convert format string with args to std::string
std::string toString(const char* format, va_list args);

// NOLINTBEGIN
inline std::string toString(const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string result = toString(format, args);
    va_end(args);
    return result;
}

// NOLINTEND

}  // namespace opcua::detail
