#pragma once

#include <cassert>
#include <optional>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/types/Builtin.h"  // StatusCode

namespace opcua {

/**
 * Represents a bad result stored in `Result`.
 */
class BadResult {
public:
    constexpr explicit BadResult(StatusCode code) noexcept
        : code_(code) {
        assert(code_.isBad());
    }

    constexpr StatusCode code() const noexcept {
        return code_;
    }

private:
    StatusCode code_;
};

/**
 * The template class Result encapsulates a status code and optionally a value.
 *
 * A result may have one of the following contents:
 * - a value and a good or uncertain status code
 * - no value and a bad status code
 *
 * The design is inspired by:
 * - C++ 23's `std::expected`: https://en.cppreference.com/w/cpp/utility/expected
 * - Rust's `Result`: https://doc.rust-lang.org/std/result
 * - Swift's `Result`: https://developer.apple.com/documentation/swift/result
 */
template <typename T>
class Result {
public:
    using ValueType = T;

    /**
     * Default constructor (default-initialized value and good status code).
     */
    constexpr Result() noexcept(std::is_nothrow_default_constructible_v<T>)
        : code_(UA_STATUSCODE_GOOD),
          maybeValue_({}) {}

    // NOLINTBEGIN(*-explicit-conversions)

    /**
     * Construct a Result with a value and a status code (good or uncertain).
     */
    constexpr Result(
        const T& value, StatusCode code = UA_STATUSCODE_GOOD
    ) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : code_(code),
          maybeValue_(value) {
        assert(!code_.isBad());
    }

    /**
     * Construct a Result with a value and a status code (good or uncertain).
     */
    constexpr Result(
        T&& value, StatusCode code = UA_STATUSCODE_GOOD
    ) noexcept(std::is_nothrow_move_constructible_v<T>)
        : code_(code),
          maybeValue_(std::move(value)) {
        assert(!code_.isBad());
    }

    /**
     * Create a Result with the given error.
     */
    constexpr Result(BadResult error) noexcept
        : code_(error.code()),
          maybeValue_(std::nullopt) {}

    operator T&() {
        return value();
    }

    operator const T&() const {
        return value();
    }

    // NOLINTEND(*-explicit-conversions)

    /**
     * Get the value of the Result.
     * Accessing a Result without a value leads to undefined behavior.
     */
    constexpr const T* operator->() const noexcept {
        return &(*maybeValue_);
    }

    /// @copydoc operator->
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

    /// @copydoc operator*
    constexpr T& operator*() & noexcept {
        return *maybeValue_;
    }

    /// @copydoc operator*
    constexpr const T&& operator*() const&& noexcept {
        return std::move(*maybeValue_);
    }

    /// @copydoc operator*
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
     * @exception BadStatus If the Result does not have a value (bad status code).
     */
    constexpr const T& value() const& {
        checkIsBad();
        return **this;
    }

    /// @copydoc value
    constexpr T& value() & {
        checkIsBad();
        return **this;
    }

    /// @copydoc value
    constexpr const T&& value() const&& {
        checkIsBad();
        return std::move(**this);
    }

    /// @copydoc value
    constexpr T&& value() && {
        checkIsBad();
        return std::move(**this);
    }

    /**
     * Get the value of the Result or a default value.
     * The default value is returned in case of an bad status code.
     */
    template <typename U>
    constexpr T valueOr(U&& defaultValue) const& {
        return !isBad() ? **this : static_cast<T>(std::forward<U>(defaultValue));
    }

    /// @copydoc valueOr
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

    StatusCode code_;
    std::optional<T> maybeValue_;
};

/**
 * Template specialization of Result class for `void` types.
 * Result<void> contains only a status code.
 */
template <>
class Result<void> {
public:
    /**
     * Create a default Result (good status code).
     */
    constexpr Result() noexcept
        : code_(UA_STATUSCODE_GOOD) {}

    /**
     * Create a Result with the given code.
     */
    constexpr Result(StatusCode code) noexcept  // NOLINT, implicit wanted
        : code_(code) {}

    /**
     * Create a Result with the given error.
     */
    constexpr Result(BadResult error) noexcept  // NOLINT, implicit wanted
        : code_(error.code()) {}

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

private:
    StatusCode code_;
};

}  // namespace opcua
