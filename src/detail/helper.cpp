#include "open62541pp/detail/helper.h"

#include <cstring>
#include <cstdio>  // vsnprintf

namespace opcua::detail {

UA_String toNativeString(std::string_view src) noexcept {
    UA_String s{src.size(), nullptr};
    if (src.data() == nullptr) {
        return s;
    }
    if (!src.empty()) {
        s.data = (UA_Byte*)src.data();  // NOLINT
    } else {
        s.data = static_cast<UA_Byte*>(UA_EMPTY_ARRAY_SENTINEL);  // NOLINT
    }
    return s;
}

UA_String allocNativeString(std::string_view src) {
    UA_String s{src.size(), nullptr};
    if (src.data() == nullptr) {
        return s;
    }
    if (!src.empty()) {
        s.data = static_cast<UA_Byte*>(UA_malloc(s.length));  // NOLINT
        std::memcpy(s.data, src.data(), src.size());
    } else {
        s.data = static_cast<UA_Byte*>(UA_EMPTY_ARRAY_SENTINEL);  // NOLINT
    }
    return s;
}

std::string toString(const char* format, va_list args) {
    // NOLINTBEGIN
    va_list argsCopy{};
    va_copy(argsCopy, args);
    const int charsToWrite = std::vsnprintf(nullptr, 0, format, argsCopy);
    va_end(argsCopy);
    // NOLINTEND
    if (charsToWrite < 0) {
        return {};
    }
    std::string buffer(charsToWrite, '\0');
    const int charsWritten = std::vsnprintf(buffer.data(), buffer.size() + 1, format, args);
    if (charsWritten < 0 || charsWritten > charsToWrite) {
        return {};
    }
    return buffer;
}

}  // namespace opcua::detail
