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
    constexpr Result() noexcept = default;

    // NOLINTNEXTLINE, implicit wanted
    constexpr Result(opcua::StatusCode code) noexcept
        : code_(code) {}

    // NOLINTNEXTLINE, implicit wanted
    operator StatusCode() {
        return code_;
    }

    // NOLINTNEXTLINE, implicit wanted
    operator UA_StatusCode() {
        return code_;
    }

    constexpr void operator*() const noexcept {}

    constexpr StatusCode code() const noexcept {
        return code_;
    }

    constexpr void value() const& {
        checkIsBad();
    }

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
 * Result encapsulates a result value and a status code.
 * A result may contain just an error status code, just the return value or both.
 */
template <typename T>
class Result {
public:
    constexpr Result() noexcept = default;

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
          maybeValue_(value) {}

    constexpr Result(StatusCode code, T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : code_(code),
          maybeValue_(std::move(value)) {}

    // TODO:
    // Think about conversion operator to value

    constexpr const T* operator->() const noexcept {
        return &(*maybeValue_);
    }

    constexpr T* operator->() noexcept {
        return &(*maybeValue_);
    }

    constexpr const T& operator*() const& noexcept {
        return *maybeValue_;
    }

    constexpr T& operator*() & noexcept {
        return *maybeValue_;
    }

    constexpr const T&& operator*() const&& noexcept {
        return std::move(*maybeValue_);
    }

    constexpr T&& operator*() && noexcept {
        return std::move(*maybeValue_);
    }

    constexpr StatusCode code() const noexcept {
        return code_;
    }

    bool hasValue() const noexcept {
        return maybeValue_.has_value();
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
