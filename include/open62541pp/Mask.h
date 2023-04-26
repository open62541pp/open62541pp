#pragma once

#include <initializer_list>
#include <type_traits>

namespace opcua {

/**
 * Bit mask using (scoped) enums as flags.
 * This class allows scoped enums to be used interchangeable with the native `UA_*` enums.
 * Additional methods to test/set/reset/flip bits similar to `std::bitset`.
 */
template <typename T>
class Mask {
public:
    static_assert(std::is_enum_v<T>, "Template type must be an enumeration type");

    /// Underlying type of enum.
    using U = std::underlying_type_t<T>;

    /// Create empty mask.
    constexpr Mask() = default;

    /// Create mask with single flag.
    constexpr Mask(T flag)  // NOLINT
        : mask_(static_cast<U>(flag)) {}

    /// Create mask with flag(s) of underlying type.
    constexpr Mask(U flag)  // NOLINT
        : mask_(flag) {}

    /// Create mask with OR-combined flags.
    constexpr Mask(std::initializer_list<T> flags) {  // NOLINT
        for (auto flag : flags) {
            set(flag);
        }
    }

    /// Get mask as underlying type.
    constexpr auto get() const noexcept {
        return mask_;
    }

    /// Implicit conversion to underlying type.
    constexpr operator U() noexcept {  // NOLINT
        return get();
    }

    /// Set a specific flag (underlying type) to 1.
    constexpr void set(U flag) noexcept {
        mask_ |= flag;
    }

    /// Set a specific flag to 1.
    constexpr void set(T flag) noexcept {
        set(static_cast<U>(flag));
    }

    /// Reset all flags to 0.
    constexpr void reset() noexcept {
        mask_ = nullValue;
    }

    /// Reset a specific flag (underlying type) to 0.
    constexpr void reset(U flag) noexcept {
        mask_ &= ~flag;
    }

    /// Reset a specific flag to 0.
    constexpr void reset(T flag) noexcept {
        reset(static_cast<U>(flag));
    }

    /// Flip all bits.
    constexpr void flip() noexcept {
        mask_ = ~mask_;
    }

    /// Test if a specific flag (underlying type) is set.
    constexpr bool test(U flag) const noexcept {
        return (mask_ & flag) != nullValue;
    }

    /// Test if a specific flag is set.
    constexpr bool test(T flag) const noexcept {
        return test(static_cast<U>(flag));
    }

    /// Check if no flags are set.
    constexpr bool none() const noexcept {
        return mask_ == nullValue;
    }

    /// Check if any flags are set.
    constexpr bool any() const noexcept {
        return mask_ != nullValue;
    }

    /// Check if all flags are set.
    constexpr bool all() const noexcept {
        constexpr U all = ~nullValue;
        return mask_ == all;
    }

private:
    static constexpr U nullValue{};
    U mask_{};
};

/* ---------------------------------------------------------------------------------------------- */

template <typename T>
constexpr Mask<T> operator|(Mask<T> left, Mask<T> right) {
    return left.get() | right.get();
}

template <typename T, typename TFlag>
constexpr Mask<T> operator|(Mask<T> mask, TFlag flag) {
    using U = typename Mask<T>::U;
    return mask.get() | static_cast<U>(flag);
}

template <typename T, typename TFlag>
constexpr Mask<T> operator|(TFlag flag, Mask<T> mask) {
    return mask | flag;
}

template <typename T, typename TFlag>
constexpr Mask<T>& operator|=(Mask<T>& mask, TFlag flag) {
    mask.set(flag);
    return mask;
}

template <typename T, typename TFlag>
constexpr bool operator==(Mask<T> mask, TFlag flag) noexcept {
    using U = typename Mask<T>::U;
    return mask.get() == static_cast<U>(flag);
}

template <typename T, typename TFlag>
constexpr bool operator==(TFlag flag, Mask<T> mask) noexcept {
    return mask == flag;
}

template <typename T, typename TFlag>
constexpr bool operator!=(Mask<T> mask, TFlag flag) noexcept {
    return !(mask == flag);
}

template <typename T, typename TFlag>
constexpr bool operator!=(TFlag flag, Mask<T> mask) noexcept {
    return !(mask == flag);
}

}  // namespace opcua
