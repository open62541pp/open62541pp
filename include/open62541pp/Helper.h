#pragma once

#include <cstring>
#include <string>
#include <string_view>

#include "open62541pp/open62541.h"

namespace opcua {

inline UA_String allocUaString(const std::string& src) noexcept {
    return UA_String_fromChars(src.c_str());
}

inline UA_String allocUaString(std::string_view src) noexcept {
    UA_String s;
    s.length = src.size();
    s.data = nullptr;

    if (!src.empty()) {
        s.data = (UA_Byte*)UA_malloc(s.length);  // NOLINT
        std::memcpy(s.data, src.data(), src.size());
    } else {
        s.data = static_cast<UA_Byte*>(UA_EMPTY_ARRAY_SENTINEL);  // NOLINT
    }
    return s;
}

inline std::string uaStringToString(const UA_String& src) noexcept {
    if (src.data == nullptr) {
        return {};
    }
    return {(char*)src.data, src.length};  // NOLINT
}

inline std::string_view uaStringToStringView(const UA_String& src) noexcept {
    if (src.data == nullptr) {
        return {};
    }
    return {(char*)src.data, src.length};  // NOLINT
}

}  // namespace opcua
