#pragma once

#include <string>
#include <string_view>

#include "open62541/types.h"
#include "open62541/types_generated_handling.h"

namespace opcua {

inline std::string uaStringToString(const UA_String& input) noexcept {
    if (input.data == nullptr)
        return {};
    
    return std::string((const char*) input.data, input.length); // NOLINT
}

inline std::string_view uaStringToStringView(const UA_String& input) noexcept {
    if (input.data == nullptr)
        return {};
    
    return std::string_view((const char*) input.data, input.length); // NOLINT
}

} // namespace opcua
