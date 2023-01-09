#pragma once

#include <string>
#include <string_view>

#include "open62541pp/open62541.h"

namespace opcua::detail {

UA_String allocUaString(const std::string& src);

UA_String allocUaString(std::string_view src);

inline std::string_view toStringView(const UA_String& src) {
    if (src.data == nullptr || src.length == 0) {
        return {};
    }
    return {(char*)src.data, src.length};  // NOLINT
}

inline std::string toString(const UA_String& src) {
    return std::string(toStringView(src));
}

}  // namespace opcua::detail
