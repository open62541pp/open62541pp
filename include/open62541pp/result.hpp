#pragma once

#include <cassert>
#include <functional>  // invoke
#include <optional>
#include <type_traits>
#include <utility>  // forward, move

#include "open62541pp/exception.hpp"
#include "open62541pp/types.hpp"  // StatusCode

namespace opcua {

/**
 * Represents a bad result stored in `Result`.
 */
class BadResult {
public:
    /**
     * Construct a BadResult from a bad StatusCode.
     */
    constexpr explicit BadResult(StatusCode code) noexcept
        : code_{std::move(code)} {
        assert(code_.isBad());
    }

    /**
     * Get the StatusCode.
     */
    constexpr StatusCode code() const noexcept {
        return code_;
    }

private:
    StatusCode code_;
};

/**
 * The template class Result encapsulates a StatusCode and optionally a value.
 *
 * A result may have one of the following contents:
 * - a value and a StatusCode (usually good or uncertain)
 * - no value and a bad StatusCode
 *
 * Result<void> is a template specialization containing only a StatusCode.
 *
 * The design is inspired by:
 * - [C++ 23's `std::expected` type](https://en.cppreference.com/w/cpp/utility/expected)
 * - [Rust's `Result` type](https://doc.rust-lang.org/std/result)
 * - [Swift's `Result` enumeration](https://developer.apple.com/documentation/swift/result)
 */
template <typename T>
class [[nodiscard]] Result {
public:
    using ValueType = T;

    /**
     * Default constructor (default-initialized value and good StatusCode).
     */
    constexpr Result() noexcept(std::is_nothrow_default_constructible_v<T>)
        : code_{UA_STATUSCODE_GOOD},
          maybeValue_({}) {}

    // NOLINTBEGIN(*explicit-conversions)

    /**
     * Construct a Result from a value (lvalue) and a StatusCode.
     */
    constexpr Result(
        const T& value, StatusCode code = UA_STATUSCODE_GOOD
    ) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : code_{std::move(code)},
          maybeValue_{value} {}

    /**
     * Construct a Result from a value (rvalue) and a StatusCode.
     */
    constexpr Result(
        T&& value, StatusCode code = UA_STATUSCODE_GOOD
    ) noexcept(std::is_nothrow_move_constructible_v<T>)
        : code_{std::move(code)},
          maybeValue_{std::move(value)} {}

    /**
     * Construct a Result from a BadResult.
     */
    constexpr Result(const BadResult& error) noexcept
        : code_{error.code()},
          maybeValue_{std::nullopt} {}

    // NOLINTEND(*explicit-conversions)

    // NOLINTBEGIN(bugprone-unchecked-optional-access)

    /**
     * Get the value of the Result.
     * Accessing a Result without a value leads to undefined behavior.
     */
    constexpr T* operator->() noexcept {
        return &(*maybeValue_);
    }

    /// @copydoc operator->
    constexpr const T* operator->() const noexcept {
        return &(*maybeValue_);
    }

    /**
     * Get the value of the Result.
     * Accessing a Result without a value leads to undefined behavior.
     */
    constexpr T& operator*() & noexcept {
        return *maybeValue_;
    }

    /// @copydoc operator*
    constexpr const T& operator*() const& noexcept {
        return *maybeValue_;
    }

    /// @copydoc operator*
    constexpr T&& operator*() && noexcept {
        return std::move(*maybeValue_);
    }

    /// @copydoc operator*
    constexpr const T&& operator*() const&& noexcept {
        return std::move(*maybeValue_);
    }

    // NOLINTEND(bugprone-unchecked-optional-access)

    /**
     * Get the StatusCode of the Result.
     */
    constexpr StatusCode code() const noexcept {
        return code_;
    }

    /**
     * Check if the Result has a value.
     */
    constexpr explicit operator bool() const noexcept {
        return hasValue();
    }

    /**
     * Check if the Result has a value.
     */
    constexpr bool hasValue() const noexcept {
        return maybeValue_.has_value();
    }

    /**
     * Get the value of the Result.
     * @exception BadStatus If the Result does not have a value (bad StatusCode).
     */
    constexpr T& value() & {
        checkIsBad();
        return **this;
    }

    /// @copydoc value
    constexpr const T& value() const& {
        checkIsBad();
        return **this;
    }

    /// @copydoc value
    constexpr T&& value() && {
        checkIsBad();
        return std::move(**this);
    }

    /// @copydoc value
    constexpr const T&& value() const&& {
        checkIsBad();
        return std::move(**this);
    }

    /**
     * Get the value of the Result or a default value.
     * The default value is returned in case of an bad StatusCode.
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

    /**
     * Transforms `Result<T>` to `Result<U>` using the given value transformation function.
     * The function is only applied if the Result has a value.
     * Otherwise `Result<U>` with the same bad StatusCode is returned.
     * @param func Callable with the signature `U(T&&)`
     */
    template <typename F>
    constexpr auto transform(F&& func) & {
        return transformImpl(*this, std::forward<F>(func));
    }

    /// @copydoc transform
    template <typename F>
    constexpr auto transform(F&& func) const& {
        return transformImpl(*this, std::forward<F>(func));
    }

    /// @copydoc transform
    template <typename F>
    constexpr auto transform(F&& func) && {
        return transformImpl(std::move(*this), std::forward<F>(func));
    }

    /// @copydoc transform
    template <typename F>
    constexpr auto transform(F&& func) const&& {
        return transformImpl(std::move(*this), std::forward<F>(func));
    }

    /**
     * Transforms `Result<T>` to `Result<U>` using the given function.
     * The function is only applied if the Result has a value.
     * Otherwise `Result<U>` with the same bad StatusCode is returned.
     * @param func Callable with the signature `Result<U>(T&&)` or `Result<U>(T&&, StatusCode)`
     */
    template <typename F>
    constexpr auto andThen(F&& func) & {
        return andThenImpl(*this, std::forward<F>(func));
    }

    /// @copydoc andThen
    template <typename F>
    constexpr auto andThen(F&& func) const& {
        return andThenImpl(*this, std::forward<F>(func));
    }

    /// @copydoc andThen
    template <typename F>
    constexpr auto andThen(F&& func) && {
        return andThenImpl(std::move(*this), std::forward<F>(func));
    }

    /// @copydoc andThen
    template <typename F>
    constexpr auto andThen(F&& func) const&& {
        return andThenImpl(std::move(*this), std::forward<F>(func));
    }

    /**
     * Transforms `Result<T>` with a bad StatusCode to `Result<T>` using the given function.
     * The function is only applied if the Result has **no** value.
     * Otherwise the same `Result<T>` is returned.
     * @param func Callable with the signature `Result<T>(StatusCode)`
     */
    template <typename F>
    constexpr auto orElse(F&& func) & {
        return orElseImpl(*this, std::forward<F>(func));
    }

    /// @copydoc orElse
    template <typename F>
    constexpr auto orElse(F&& func) const& {
        return orElseImpl(*this, std::forward<F>(func));
    }

    /// @copydoc orElse
    template <typename F>
    constexpr auto orElse(F&& func) && {
        return orElseImpl(std::move(*this), std::forward<F>(func));
    }

    /// @copydoc orElse
    template <typename F>
    constexpr auto orElse(F&& func) const&& {
        return orElseImpl(std::move(*this), std::forward<F>(func));
    }

private:
    template <typename Self, typename F>
    static auto transformImpl(Self&& self, F&& func) {
        using Value = decltype(*std::forward<Self>(self));
        using NewValue = std::remove_cv_t<std::invoke_result_t<F, Value>>;
        if (self.hasValue()) {
            if constexpr (std::is_void_v<NewValue>) {
                return Result<NewValue>();
            } else {
                return Result<NewValue>(
                    std::invoke(std::forward<F>(func), *std::forward<Self>(self)), self.code()
                );
            }
        } else {
            return Result<NewValue>(BadResult(self.code()));
        }
    }

    template <typename Self, typename F>
    static auto andThenImpl(Self&& self, F&& func) {
        using Value = decltype(*std::forward<Self>(self));
        if constexpr (std::is_invocable_v<F, Value, StatusCode>) {
            using NewResult = std::remove_cv_t<std::invoke_result_t<F, Value, StatusCode>>;
            return self.hasValue()
                ? std::invoke(std::forward<F>(func), *std::forward<Self>(self), self.code())
                : NewResult(BadResult(self.code()));
        } else {
            using NewResult = std::remove_cv_t<std::invoke_result_t<F, Value>>;
            return self.hasValue()
                ? std::invoke(std::forward<F>(func), *std::forward<Self>(self))
                : NewResult(BadResult(self.code()));
        }
    }

    template <typename Self, typename F>
    static auto orElseImpl(Self&& self, F&& func) {
        using NewResult = std::remove_cv_t<std::invoke_result_t<F, decltype(self.code())>>;
        return self.hasValue()
            ? std::forward<Self>(self)
            : NewResult(std::invoke(std::forward<F>(func), self.code()));
    }

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
 * Result<void> contains only a StatusCode.
 */
template <>
class [[nodiscard]] Result<void> {
public:
    /**
     * Create a default Result (good StatusCode).
     */
    constexpr Result() noexcept
        : code_{UA_STATUSCODE_GOOD} {}

    /**
     * Create a Result with the given StatusCode.
     */
    constexpr Result(StatusCode code) noexcept  // NOLINT(*explicit-conversions)
        : code_{std::move(code)} {}

    /**
     * Create a Result with the given error.
     */
    constexpr Result(const BadResult& error) noexcept  // NOLINT(*explicit-conversions)
        : code_{error.code()} {}

    constexpr void operator*() const noexcept {}

    /**
     * Get the code of the Result.
     */
    constexpr StatusCode code() const noexcept {
        return code_;
    }

    /**
     * Check if the Result has a non-bad StatusCode.
     */
    constexpr explicit operator bool() const noexcept {
        return hasValue();
    }

    /*
     * Check if the Result has a non-bad StatusCode.
     */
    constexpr bool hasValue() const noexcept {
        return !code_.isBad();
    }

    /**
     * Get the value of the Result.
     * @exception BadStatus If the Result does have a bad StatusCode.
     */
    constexpr void value() const {
        code().throwIfBad();
    }

private:
    StatusCode code_;
};

}  // namespace opcua
