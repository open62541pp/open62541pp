#pragma once

#include <functional>  // invoke
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
    using runtime_error::runtime_error;  // inherit contructors
};

class CreateCertificateError : public std::runtime_error {
public:
    using runtime_error::runtime_error;  // inherit contructors
};

namespace detail {

[[nodiscard]] inline constexpr bool isGoodStatus(UA_StatusCode code) noexcept {
    return code == UA_STATUSCODE_GOOD;
}

[[nodiscard]] inline constexpr bool isBadStatus(UA_StatusCode code) noexcept {
    return code != UA_STATUSCODE_GOOD;
}

inline void throwOnBadStatus(UA_StatusCode code) {
    if (isBadStatus(code)) {
        // NOLINTNEXTLINE
        switch (code) {
        case UA_STATUSCODE_BADDISCONNECT:
            throw BadDisconnect();
        default:
            throw BadStatus(code);
        }
    }
}

/**
 * Invoke a function (with `void` return type), catch and ignore exceptions.
 * This is especially useful for C-API callbacks, that are executed within the open62541 event loop.
 */
template <typename F, typename... Args>
void invokeCatchIgnore(F&& fn, Args&&... args) noexcept {
    try {
        std::invoke(fn, std::forward<Args>(args)...);
    } catch (...) {
        // ignore
    }
}

/**
 * Invoke a function (with `void` return type) and catch exceptions.
 * This is especially useful for C-API callbacks, that are executed within the open62541 event loop.
 * If no exception is thrown, `UA_STATUSCODE_GOOD` is returned.
 * If the exception if of type BadStatus, the underlying status code will be returned.
 * All other exception types will yield `UA_STATUSCODE_BADINTERNALERROR`.
 */
template <typename F, typename... Args>
[[nodiscard]] UA_StatusCode invokeCatchStatus(F&& fn, Args&&... args) noexcept {
    try {
        std::invoke(fn, std::forward<Args>(args)...);
        return UA_STATUSCODE_GOOD;
    } catch (const BadStatus& e) {
        return e.code();
    } catch (...) {
        return UA_STATUSCODE_BADINTERNALERROR;
    }
}

}  // namespace detail

}  // namespace opcua
