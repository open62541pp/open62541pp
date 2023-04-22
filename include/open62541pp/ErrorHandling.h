#pragma once

#include <stdexcept>
#include <string>

#include "open62541pp/open62541.h"

namespace opcua {

/**
 * Exception for bad status codes from open62541 `UA_STATUSCODE_*`.
 * @see statuscodes.h
 */
class BadStatus : public std::exception {
public:
    explicit BadStatus(UA_StatusCode code)
        : code_(code) {}

    UA_StatusCode code() const noexcept {
        return code_;
    }

    const char* what() const noexcept override {
        return UA_StatusCode_name(code_);
    }

private:
    UA_StatusCode code_;
};

class BadVariantAccess : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;  // inherit contructors
};

class ClientNotConnected : public BadStatus {
public:
	explicit ClientNotConnected (UA_StatusCode code) : BadStatus(code){}
};

namespace detail {

[[nodiscard]] inline constexpr bool isGoodStatus(UA_StatusCode code) noexcept {
    return code == UA_STATUSCODE_GOOD;
}

[[nodiscard]] inline constexpr bool isBadStatus(UA_StatusCode code) noexcept {
    return code != UA_STATUSCODE_GOOD;
}

[[nodiscard]] inline constexpr bool isServerNotConnected(UA_StatusCode code) noexcept {
	return code == UA_STATUSCODE_BADSERVERNOTCONNECTED;
}

inline void throwOnBadStatus(UA_StatusCode code) {
    if (isBadStatus(code)) {
        throw BadStatus(code);
    }

	if (isServerNotConnected(code)) {
		throw ClientNotConnected(code);
	}
}

}  // namespace detail

}  // namespace opcua