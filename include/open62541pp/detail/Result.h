#pragma once

#include <cassert>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/types/Builtin.h"  // StatusCode

namespace opcua::detail {

/**
 * Represents a bad result stored in `Result`.
 */
class BadResult {
public:
    constexpr explicit BadResult(StatusCode code) noexcept
        : code_(code) {
        assert(code.isBad());
    }

    constexpr StatusCode code() const noexcept {
        return code_;
    }

private:
    StatusCode code_;
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
    constexpr Result(BadResult error) noexcept
        : code_(error.code()) {}

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

    constexpr StatusCode code() const noexcept {
        return code_;
    }

    constexpr bool isGood() const noexcept {
        return code().isGood();
    }

    constexpr bool isBad() const noexcept {
        return code().isBad();
    }

    constexpr const T& value() const& {
        checkIsBad();
        return **this;
    }

    constexpr T& value() & {
        checkIsBad();
        return **this;
    }

    constexpr const T&& value() const&& {
        checkIsBad();
        return std::move(**this);
    }

    constexpr T&& value() && {
        checkIsBad();
        return std::move(**this);
    }

    template <typename U>
    constexpr T valueOr(U&& defaultValue) const& {
        return !isBad() ? **this : static_cast<T>(std::forward<U>(defaultValue));
    }

    template <typename U>
    constexpr T valueOr(U&& defaultValue) && {
        return !isBad() ? std::move(**this) : static_cast<T>(std::forward<U>(defaultValue));
    }

private:
    constexpr void checkIsBad() const {
        code().throwIfBad();
    }

    StatusCode code_{};
    T value_{};
};

template <>
class Result<void> {
public:
    constexpr Result() noexcept = default;

    // NOLINTNEXTLINE, implicit wanted
    constexpr Result(BadResult error) noexcept
        : code_(error.code()) {}

    constexpr const void* operator->() const noexcept {
        return nullptr;
    }

    constexpr void* operator->() noexcept {
        return nullptr;
    }

    constexpr void operator*() const noexcept {}

    constexpr StatusCode code() const noexcept {
        return code_;
    }

    constexpr bool isGood() const noexcept {
        return code().isGood();
    }

    constexpr bool isBad() const noexcept {
        return code().isBad();
    }

    constexpr void value() const {
        code().throwIfBad();
    }

private:
    StatusCode code_{};
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
    } catch (...) {
        return BadResult(getStatusCode(std::current_exception()));
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
