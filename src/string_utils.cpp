#include "open62541pp/detail/string_utils.hpp"

#include <cstring>

#include "open62541pp/exception.hpp"

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

[[nodiscard]] UA_String allocNativeString(std::string_view src) {
    UA_String s{src.size(), nullptr};
    if (src.data() == nullptr) {
        return s;
    }
    if (!src.empty()) {
        s.data = static_cast<UA_Byte*>(UA_malloc(s.length));  // NOLINT
        if (s.data == nullptr) {
            throw BadStatus(UA_STATUSCODE_BADOUTOFMEMORY);
        }
        std::memcpy(s.data, src.data(), src.size());
    } else {
        s.data = static_cast<UA_Byte*>(UA_EMPTY_ARRAY_SENTINEL);  // NOLINT
    }
    return s;
}

[[nodiscard]] char* allocCString(std::string_view src) {
    char* cstr = static_cast<char*>(UA_malloc(src.size() + 1));  // NOLINT
    if (cstr == nullptr) {
        throw BadStatus(UA_STATUSCODE_BADOUTOFMEMORY);
    }
    std::strncpy(cstr, src.data(), src.size());
    cstr[src.size()] = '\0';  // NOLINT
    return cstr;
}

void clear(const char* str) noexcept {
    UA_free((void*)str);  // NOLINT
}

}  // namespace opcua::detail
