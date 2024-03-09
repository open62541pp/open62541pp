#pragma once

#include <cassert>
#include <optional>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/types/Builtin.h"  // StatusCode

namespace opcua {

template <typename T>
class Result;

/**
 * Result<void> type.
 * A result with void specialization contains only a status code.
 */
template <>
class Result<void> {
public:
    /**
     * Create a default Result (good status code).
     */
    constexpr Result() noexcept = default;

    /**
     * Create a Result with the given code.
     */
    constexpr Result(opcua::StatusCode code) noexcept  // NOLINT, implicit wanted
        : code_(code) {}

    /**
     * Convert the Result to a StatusCode.
     */
    operator StatusCode() const noexcept {  // NOLINT, implicit wanted
        return code_;
    }

    constexpr void operator*() const noexcept {}

    /**
     * Get the code of the Result.
     */
    constexpr StatusCode code() const noexcept {
        return code_;
    }

    /**
     * Get the value of the Result.
     */
    constexpr void value() const& {
        checkIsBad();
    }

    /**
     * Get the value of the Result.
     */
    constexpr void value() && {
        checkIsBad();
    }

private:
    constexpr void checkIsBad() const {
        code().throwIfBad();
    }

    StatusCode code_{};
};

/**
 * Represents a bad result stored in `Result`.
 */
class BadResult : public Result<void> {
public:
    constexpr explicit BadResult(StatusCode code) noexcept
        : Result(code) {
        assert(code.isBad());
    }
};

/**
 * Result encapsulates a status code and a result value.
 * A result may have one of the following contents:
 * - just an error status code
 * - a good status code and a return value
 * - an uncertain status code and a return value
 */
template <typename T>
class Result {
public:
    using ValueType = T;

#ifdef _MSC_VER
    // Workaround required for MSVC
    // (https://developercommunity.visualstudio.com/t/c-shared-state-futuresstate-default-constructs-the/60897).
    constexpr Result() noexcept
        : code_(UA_STATUSCODE_BADINTERNALERROR) {}

#endif

    // NOLINTNEXTLINE, implicit wanted
    constexpr Result(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : maybeValue_(value) {}

    // NOLINTNEXTLINE, implicit wanted
    constexpr Result(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : maybeValue_(std::move(value)) {}

    // NOLINTNEXTLINE, implicit wanted
    constexpr Result(BadResult error) noexcept
        : code_(error.code()) {}

    constexpr Result(
        StatusCode code, const T& value
    ) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : code_(code),
          maybeValue_(value) {
        assert(!code.isBad());
    }

    constexpr Result(StatusCode code, T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : code_(code),
          maybeValue_(std::move(value)) {
        assert(!code.isBad());
    }

    // NOLINTNEXTLINE, implicit wanted
    operator T&() {
        return value();
    }

    // NOLINTNEXTLINE, implicit wanted
    operator const T&() const {
        return value();
    }

    /**
     * Get the value of the Result.
     * Accessing a Result without a value leads to undefined behavior.
     */
    constexpr const T* operator->() const noexcept {
        return &(*maybeValue_);
    }

    /**
     * Get the value of the Result.
     * Accessing a Result without a value leads to undefined behavior.
     */
    constexpr T* operator->() noexcept {
        return &(*maybeValue_);
    }

    /**
     * Get the value of the Result.
     * Accessing a Result without a value leads to undefined behavior.
     */
    constexpr const T& operator*() const& noexcept {
        return *maybeValue_;
    }

    /**
     * Get the value of the Result.
     * Accessing a Result without a value leads to undefined behavior.
     */
    constexpr T& operator*() & noexcept {
        return *maybeValue_;
    }

    /**
     * Get the value of the Result.
     * Accessing a Result without a value leads to undefined behavior.
     */
    constexpr const T&& operator*() const&& noexcept {
        return std::move(*maybeValue_);
    }

    /**
     * Get the value of the Result.
     * Accessing a Result without a value leads to undefined behavior.
     */
    constexpr T&& operator*() && noexcept {
        return std::move(*maybeValue_);
    }

    /**
     * Get the code of the Result.
     */
    constexpr StatusCode code() const noexcept {
        return code_;
    }

    /**
     * Check if the Result has a value.
     */
    bool hasValue() const noexcept {
        return maybeValue_.has_value();
    }

    /**
     * Get the value of the Result.
     */
    constexpr const T& value() const& {
        checkIsBad();
        return **this;
    }

    /**
     * Get the value of the Result.
     */
    constexpr T& value() & {
        checkIsBad();
        return **this;
    }

    /**
     * Get the value of the Result.
     */
    constexpr const T&& value() const&& {
        checkIsBad();
        return std::move(**this);
    }

    /**
     * Get the value of the Result.
     */
    constexpr T&& value() && {
        checkIsBad();
        return std::move(**this);
    }

    /**
     * Get the value of the Result or a default value.
     * The default value is returned in case of an error status code.
     */
    template <typename U>
    constexpr T valueOr(U&& defaultValue) const& {
        return !isBad() ? **this : static_cast<T>(std::forward<U>(defaultValue));
    }

    /**
     * Get the value of the Result or a default value.
     * The default value is returned in case of an error status code.
     */
    template <typename U>
    constexpr T valueOr(U&& defaultValue) && {
        return !isBad() ? std::move(**this) : static_cast<T>(std::forward<U>(defaultValue));
    }

private:
    constexpr bool isBad() const noexcept {
        return code().isBad();
    }

    constexpr void checkIsBad() const {
        code().throwIfBad();
    }

    StatusCode code_{};
    std::optional<T> maybeValue_{};
};

}  // namespace opcua
