#pragma once

#include <string>
#include <string_view>
#include <stdexcept>

#include "open62541pp/open62541.h"

namespace opcua {

class Exception : public std::runtime_error {
public:
    explicit Exception(UA_StatusCode statusCode)
        : std::runtime_error(getStatusMessage(statusCode)) {}
    
    explicit Exception(std::string_view message)
        : std::runtime_error(message.data()) {}  
    
    inline std::string getStatusMessage(UA_StatusCode statusCode) const noexcept {
        std::string msg {"OPC UA error: "};
        msg += UA_StatusCode_name(statusCode);
        return msg;
    }
};

inline bool checkStatusCode(UA_StatusCode code) noexcept {
    if (code != UA_STATUSCODE_GOOD) {
        return false;
    }
    return true;
}

inline void checkStatusCodeException(UA_StatusCode code) {
    if (code != UA_STATUSCODE_GOOD) {
        throw Exception(code);
    }
}

} // namespace opcua
