#pragma once

#include <cassert>
#include <exception>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/types/Builtin.h"  // StatusCode

namespace opcua::detail {

/**
 * Represents a bad result stored in `Result`.
 */
class BadResult {
public:
    explicit BadResult(StatusCode code) noexcept
        : code_(code) {
        assert(code_.isBad());
    }

    explicit BadResult(std::exception_ptr exception) noexcept
        : code_(UA_STATUSCODE_BAD),
          exception_(std::move(exception)) {  // NOLINT
        assert(exception_ != nullptr);
    }

    constexpr const std::exception_ptr& exception() const& noexcept {
        return exception_;
    }

    constexpr std::exception_ptr& exception() & noexcept {
        return exception_;
    }

    constexpr const std::exception_ptr&& exception() const&& noexcept {
        return std::move(exception_);  // NOLINT
    }

    constexpr std::exception_ptr&& exception() && noexcept {
        return std::move(exception_);
    }

    constexpr StatusCode code() const noexcept {
        return code_;
    }

private:
    StatusCode code_{};
    std::exception_ptr exception_{};
};

/**
 * Result encapsulates a result value and a status code.
 * A result may contain just an error status code, just the return value or both.
 */
template <typename T>
class Result {
public:
    constexpr Result() noexcept(std::is_nothrow_default_constructible_v<T>) = default;

    // NOLINTNEXTLINE, implicit wanted
    constexpr Result(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : value_(value) {}

    // NOLINTNEXTLINE, implicit wanted
    constexpr Result(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : value_(std::move(value)) {}

    // NOLINTNEXTLINE, implicit wanted
    Result(BadResult error) noexcept
        : code_(error.code()),
          exception_(std::move(error).exception()) {}

    constexpr Result(
        StatusCode code, const T& value
    ) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : code_(code),
          value_(value) {}

    constexpr Result(StatusCode code, T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : code_(code),
          value_(std::move(value)) {}

    constexpr const T* operator->() const noexcept {
        return &value_;
    }

    constexpr T* operator->() noexcept {
        return &value_;
    }

    constexpr const T& operator*() const& noexcept {
        return value_;
    }

    constexpr T& operator*() & noexcept {
        return value_;
    }

    constexpr const T&& operator*() const&& noexcept {
        return std::move(value_);
    }

    constexpr T&& operator*() && noexcept {
        return std::move(value_);
    }

    bool hasException() const noexcept {
        return exception_ != nullptr;
    }

    constexpr const std::exception_ptr& exception() const noexcept {
        return exception_;
    }

    constexpr StatusCode code() const noexcept {
        return code_;
    }

    constexpr const T& value() const& {
        throwExceptionOrBadStatus();
        return **this;
    }

    constexpr T& value() & {
        throwExceptionOrBadStatus();
        return **this;
    }

    constexpr const T&& value() const&& {
        throwExceptionOrBadStatus();
        return std::move(**this);
    }

    constexpr T&& value() && {
        throwExceptionOrBadStatus();
        return std::move(**this);
    }

    template <typename U>
    constexpr T valueOr(U&& defaultValue) const& {
        return isGood() ? **this : static_cast<T>(std::forward<U>(defaultValue));
    }

    template <typename U>
    constexpr T valueOr(U&& defaultValue) && {
        return isGood() ? std::move(**this) : static_cast<T>(std::forward<U>(defaultValue));
    }

private:
    constexpr bool isGood() const {
        return !hasException() && !code().isBad();
    }

    constexpr void throwExceptionOrBadStatus() const {
        if (hasException()) {
            std::rethrow_exception(exception());
        }
        code().throwIfBad();
    }

    StatusCode code_{};
    std::exception_ptr exception_{};
    T value_{};
};

template <>
class Result<void> {
public:
    constexpr Result() noexcept = default;

    // NOLINTNEXTLINE, implicit wanted
    Result(BadResult error) noexcept
        : code_(error.code()),
          exception_(std::move(error).exception()) {}

    constexpr const void* operator->() const noexcept {
        return nullptr;
    }

    constexpr void* operator->() noexcept {
        return nullptr;
    }

    constexpr void operator*() const noexcept {}

    bool hasException() const noexcept {
        return exception_ != nullptr;
    }

    constexpr const std::exception_ptr& exception() const noexcept {
        return exception_;
    }

    constexpr StatusCode code() const noexcept {
        return code_;
    }

    constexpr void value() const {
        code().throwIfBad();
    }

private:
    StatusCode code_{};
    std::exception_ptr exception_{};
};

/**
 * Invoke a function and capture its Result (value or status code).
 * This is especially useful for C-API callbacks, that are executed within the open62541 event loop.
 */
template <typename F, typename... Args>
auto tryInvoke(F&& func, Args&&... args) noexcept -> Result<std::invoke_result_t<F, Args...>> {
    using ReturnType = std::invoke_result_t<F, Args...>;
    try {
        if constexpr (std::is_void_v<ReturnType>) {
            std::invoke(std::forward<F>(func), std::forward<Args>(args)...);
            return {};
        } else {
            return std::invoke(std::forward<F>(func), std::forward<Args>(args)...);
        }
    } catch (const BadStatus& e) {
        return BadResult(e.code());
    } catch (...) {
        return BadResult(std::current_exception());
    }
}

/**
 * Invoke a function and get the status code of the operation.
 * This is especially useful for C-API callbacks, that are executed within the open62541 event loop.
 */
template <typename F, typename... Args>
StatusCode tryInvokeGetStatus(F&& func, Args&&... args) noexcept {
    using ReturnType = std::invoke_result_t<F, Args...>;
    auto result = tryInvoke(std::forward<F>(func), std::forward<Args>(args)...);
    if constexpr (std::is_same_v<ReturnType, StatusCode> || std::is_same_v<ReturnType, UA_StatusCode>) {
        return result.valueOr(result.code());
    } else {
        return result.code();
    }
}

}  // namespace opcua::detail
