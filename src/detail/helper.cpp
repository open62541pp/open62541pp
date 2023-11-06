#include "open62541pp/detail/helper.h"

#include <cstring>

namespace opcua::detail {

UA_String toUaString(std::string_view src) noexcept {
    UA_String s{src.size(), nullptr};
    if (!src.empty()) {
        s.data = (UA_Byte*)src.data();  // NOLINT
    } else {
        s.data = static_cast<UA_Byte*>(UA_EMPTY_ARRAY_SENTINEL);  // NOLINT
    }
    return s;
}

UA_String allocUaString(std::string_view src) {
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

}  // namespace opcua::detail
