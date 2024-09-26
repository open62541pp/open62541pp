#pragma once

#include <exception>
#include <new>  // bad_alloc
#include <stdexcept>

#include "open62541pp/detail/open62541/common.h"

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

/**
 * Specific exception for open62541 status code `UA_STATUSCODE_BADDISCONNECT`.
 * Useful to catch Client disconnects.
 */
class BadDisconnect : public BadStatus {
public:
    BadDisconnect()
        : BadStatus(UA_STATUSCODE_BADDISCONNECT) {}
};

class BadVariantAccess : public std::runtime_error {
public:
    using runtime_error::runtime_error;  // inherit constructors
};

class CreateCertificateError : public std::runtime_error {
public:
    using runtime_error::runtime_error;  // inherit constructors
};

namespace detail {

[[nodiscard]] constexpr bool isGood(UA_StatusCode code) noexcept {
    return (code >> 30U) == 0x00;
}

[[nodiscard]] constexpr bool isUncertain(UA_StatusCode code) noexcept {
    return (code >> 30U) == 0x01;
}

[[nodiscard]] constexpr bool isBad(UA_StatusCode code) noexcept {
    return (code >> 30U) >= 0x02;
}

// NOLINTNEXTLINE, pass by value ok
[[nodiscard]] inline UA_StatusCode getStatusCode(std::exception_ptr eptr) noexcept {
    try {
        if (eptr) {
            std::rethrow_exception(eptr);
        }
    } catch (const BadStatus& e) {
        return e.code();
    } catch (const std::bad_alloc& /*e*/) {
        return UA_STATUSCODE_BADOUTOFMEMORY;
    } catch (...) {
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    return UA_STATUSCODE_GOOD;
}

}  // namespace detail

/**
 * Check the status code and throw a BadStatus exception if the status code is bad.
 */
constexpr void throwIfBad(UA_StatusCode code) {
    if (detail::isBad(code)) {
        // NOLINTNEXTLINE
        switch (code) {
        case UA_STATUSCODE_BADDISCONNECT:
            throw BadDisconnect();
        default:
            throw BadStatus(code);
        }
    }
}

}  // namespace opcua
