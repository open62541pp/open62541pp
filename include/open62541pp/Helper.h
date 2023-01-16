#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "open62541pp/Types.h"
#include "open62541pp/open62541.h"

namespace opcua::detail {

/// Get UA_DataType by Type enum.
inline const UA_DataType* getUaDataType(Type type) noexcept {
    const auto typeIndex = static_cast<uint16_t>(type);
    if (typeIndex < UA_TYPES_COUNT) {
        return &UA_TYPES[typeIndex];  // NOLINT
    }
    return nullptr;
}

/// Get (custom) UA_DataType by UA_NodeId.
/// Return nullptr if no matching data type was found.
inline const UA_DataType* getUaDataType(const UA_NodeId* id) noexcept {
    return UA_findDataType(id);
}

/// Allocate UA_String from const char*
UA_String allocUaString(const char* src);

/// Allocate UA_String from std::string
UA_String allocUaString(const std::string& src);

/// Allocate UA_String from std::string_view
UA_String allocUaString(std::string_view src);

/// Convert UA_String to std::string_view
inline std::string_view toStringView(const UA_String& src) {
    if (src.data == nullptr || src.length == 0) {
        return {};
    }
    return {(char*)src.data, src.length};  // NOLINT
}

/// Convert UA_String to std::string
inline std::string toString(const UA_String& src) {
    return std::string(toStringView(src));
}

}  // namespace opcua::detail
