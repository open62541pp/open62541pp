#pragma once

#include <stdexcept>
#include <string>

#include "open62541pp/open62541.h"

namespace opcua {

class Exception : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;  // inherit contructors

    explicit Exception(UA_StatusCode statusCode)
        : std::runtime_error(getStatusMessage(statusCode)) {}

private:
    static std::string getStatusMessage(UA_StatusCode statusCode) {
        static const std::string msg{"OPC UA error: "};
        return msg + UA_StatusCode_name(statusCode);
    }
};

class InvalidNodeClass : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;  // inherit contructors
};

namespace detail {

inline constexpr bool checkStatusCode(UA_StatusCode code) noexcept {
    return code == UA_STATUSCODE_GOOD;
}

inline void checkStatusCodeException(UA_StatusCode code) {
    if (code != UA_STATUSCODE_GOOD) {
        throw Exception(code);
    }
}

}  // namespace detail

}  // namespace opcua
