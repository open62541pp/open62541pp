#pragma once

#include <functional>  // invoke
#include <stdexcept>
#include <string>
#include <type_traits>  // invoke_result_t

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
    using runtime_error::runtime_error;  // inherit constructors
};

class CreateCertificateError : public std::runtime_error {
public:
    using runtime_error::runtime_error;  // inherit constructors
};

namespace detail {

[[nodiscard]] inline constexpr bool isGoodStatus(UA_StatusCode code) noexcept {
    return (code >> 30U) == 0x00;
}

[[nodiscard]] inline constexpr bool isUncertainStatus(UA_StatusCode code) noexcept {
    return (code >> 30U) == 0x01;
}

[[nodiscard]] inline constexpr bool isBadStatus(UA_StatusCode code) noexcept {
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
    } catch (...) {
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    return UA_STATUSCODE_GOOD;
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

inline void throwOnBadStatus(const UA_StatusCode* codes, size_t codesSize) {
    for (size_t i = 0; i < codesSize; ++i) {
        throwOnBadStatus(codes[i]);  // NOLINT
    }
}

/**
 * Invoke a function (with `void` return type), catch and ignore exceptions.
 * This is especially useful for C-API callbacks, that are executed within the open62541 event loop.
 */
template <typename F, typename... Args>
void invokeCatchIgnore(F&& fn, Args&&... args) noexcept {
    using ReturnType = std::invoke_result_t<F, Args&&...>;
    static_assert(std::is_same_v<ReturnType, void>, "Only return types of type void allowed");
    try {
        std::invoke(fn, std::forward<Args>(args)...);
    } catch (...) {
        // ignore
    }
}

/**
 * Invoke a function (with `void` or `UA_StatusCode` return type) and catch exceptions.
 * This is especially useful for C-API callbacks, that are executed within the open62541 event loop.
 * If no exception is thrown, the generated status code or `UA_STATUSCODE_GOOD` is returned.
 * If the exception if of type BadStatus, the underlying status code will be returned.
 * All other exception types will yield `UA_STATUSCODE_BADINTERNALERROR`.
 */
template <typename F, typename... Args>
[[nodiscard]] UA_StatusCode invokeCatchStatus(F&& fn, Args&&... args) noexcept {
    using ReturnType = std::invoke_result_t<F, Args&&...>;
    static_assert(
        std::is_same_v<ReturnType, void> || std::is_same_v<ReturnType, UA_StatusCode>,
        "Only return types of type void or UA_StatusCode allowed"
    );
    try {
        if constexpr (std::is_same_v<ReturnType, UA_StatusCode>) {
            return std::invoke(fn, std::forward<Args>(args)...);
        } else {
            std::invoke(fn, std::forward<Args>(args)...);
            return UA_STATUSCODE_GOOD;
        }
    } catch (...) {
        return getStatusCode(std::current_exception());
    }
}

/**
 * Capture of function invocation (result, exception pointer and status code).
 * The class is specizalized for `void` return types to allow generic handling.
 * It can only be instantiated by calling `invokeCapture`.
 */
template <typename T>
class InvokeCapture {
public:
    using ResultType = T;

    explicit InvokeCapture(T result)
        : result_(std::move(result)) {}

    explicit InvokeCapture(std::exception_ptr eptr)
        : eptr_(std::move(eptr)) {}

    T& get() noexcept {
        return result_;
    }

    const T& get() const noexcept {
        return result_;
    }

    std::exception_ptr exception() const noexcept {
        return eptr_;
    }

    UA_StatusCode code() const noexcept {
        return getStatusCode(eptr_);
    }

private:
    T result_{};
    std::exception_ptr eptr_{};
};

template <>
class InvokeCapture<void> {
public:
    using ResultType = void;

    explicit InvokeCapture() = default;

    explicit InvokeCapture(std::exception_ptr eptr)
        : eptr_(std::move(eptr)) {}

    void get() const noexcept {}

    std::exception_ptr exception() const noexcept {
        return eptr_;
    }

    UA_StatusCode code() const noexcept {
        return detail::getStatusCode(eptr_);
    }

private:
    std::exception_ptr eptr_{};
};

template <typename F, typename... Args>
inline static auto invokeCapture(F&& func, Args&&... args) noexcept {
    using FuncResult = std::invoke_result_t<F, Args...>;
    using Result = InvokeCapture<FuncResult>;

    try {
        if constexpr (std::is_void_v<FuncResult>) {
            std::invoke(std::forward<F>(func), std::forward<Args>(args)...);
            return Result();
        } else {
            return Result(std::invoke(std::forward<F>(func), std::forward<Args>(args)...));
        }
    } catch (...) {
        return Result(std::current_exception());
    }
}

}  // namespace detail

}  // namespace opcua
