#pragma once

#include <type_traits>

namespace opcua {

/**
 * \addtogroup BitmaskAbstraction Bitmask abstraction
 * @{
 */

/**
 * Trait to define an enum (class) as a bitmask and allow bitwise operations.
 *
 * @code{.cpp}
 * // define enum (class)
 * enum class Access {
 *     Read  = 1 << 0,
 *     Write = 1 << 1,
 * };
 *
 * // allow bitwise operations
 * template <>
 * struct IsBitmaskEnum<Access> : std::true_type {};
 *
 * // use bitwise operations
 * Access mask = Access::Read | Access::Write;
 * @endcode
 */
template <typename T>
struct IsBitmaskEnum : std::false_type {};

/* -------------------------------------- Bitwise operators ------------------------------------- */

/// @relates IsBitmaskEnum
template <typename T>
constexpr typename std::enable_if_t<IsBitmaskEnum<T>::value, T> operator&(T lhs, T rhs) noexcept {
    using U = typename std::underlying_type_t<T>;
    return static_cast<T>(static_cast<U>(lhs) & static_cast<U>(rhs));
}

/// @relates IsBitmaskEnum
template <typename T>
constexpr typename std::enable_if_t<IsBitmaskEnum<T>::value, T> operator|(T lhs, T rhs) noexcept {
    using U = typename std::underlying_type_t<T>;
    return static_cast<T>(static_cast<U>(lhs) | static_cast<U>(rhs));
}

/// @relates IsBitmaskEnum
template <typename T>
constexpr typename std::enable_if_t<IsBitmaskEnum<T>::value, T> operator^(T lhs, T rhs) noexcept {
    using U = typename std::underlying_type_t<T>;
    return static_cast<T>(static_cast<U>(lhs) ^ static_cast<U>(rhs));
}

/// @relates IsBitmaskEnum
template <typename T>
constexpr typename std::enable_if_t<IsBitmaskEnum<T>::value, T> operator~(T rhs) noexcept {
    using U = typename std::underlying_type_t<T>;
    return static_cast<T>(~static_cast<U>(rhs));
}

/* -------------------------------- Bitwise assignment operators -------------------------------- */

/// @relates IsBitmaskEnum
template <typename T>
constexpr typename std::enable_if_t<IsBitmaskEnum<T>::value, T> operator|=(T& lhs, T rhs) noexcept {
    using U = typename std::underlying_type_t<T>;
    lhs = static_cast<T>(static_cast<U>(lhs) | static_cast<U>(rhs));
    return lhs;
}

/// @relates IsBitmaskEnum
template <typename T>
constexpr typename std::enable_if_t<IsBitmaskEnum<T>::value, T> operator&=(T& lhs, T rhs) noexcept {
    using U = typename std::underlying_type_t<T>;
    lhs = static_cast<T>(static_cast<U>(lhs) & static_cast<U>(rhs));
    return lhs;
}

/// @relates IsBitmaskEnum
template <typename T>
constexpr typename std::enable_if_t<IsBitmaskEnum<T>::value, T> operator^=(T& lhs, T rhs) noexcept {
    using U = typename std::underlying_type_t<T>;
    lhs = static_cast<T>(static_cast<U>(lhs) ^ static_cast<U>(rhs));
    return lhs;
}

/* ---------------------------------------- Bitmask class --------------------------------------- */

/**
 * Bitmask using (scoped) enums.
 * Zero-cost abstraction to specify bitmasks with enums/ints or enum classes.
 *
 * @code{.cpp}
 * // construct with scoped enums
 * Bitmask<NodeClass> mask = NodeClass::Variable | NodeClass::Object;
 * // construct with unscoped enums or ints
 * Bitmask<NodeClass> mask = UA_NODECLASS_VARIABLE | UA_NODECLASS_OBJECT;
 * @endcode
 *
 * @tparam T Enumeration type
 *
 * @see https://www.strikerx3.dev/cpp/2019/02/27/typesafe-enum-class-bitmasks-in-cpp.html
 * @see https://andreasfertig.blog/2024/01/cpp20-concepts-applied/
 */
template <typename T>
class Bitmask {
public:
    static_assert(std::is_enum_v<T>);
    static_assert(IsBitmaskEnum<T>::value);

    using Underlying = std::underlying_type_t<T>;

    /// Create an empty bitmask.
    constexpr Bitmask() noexcept = default;

    /// Create a bitmask from the enumeration type.
    constexpr Bitmask(T mask) noexcept  // NOLINT, implicit wanted
        : mask_(static_cast<Underlying>(mask)) {}

    /// Create a bitmask from the underlying type.
    constexpr Bitmask(Underlying mask) noexcept  // NOLINT, implicit wanted
        : mask_(mask) {}

    /// Conversion to the enum type.
    constexpr explicit operator T() const noexcept {
        return static_cast<T>(mask_);
    }

    /// Conversion to the underlying type.
    [[deprecated("Implicit conversion to integer will be removed. Use the get() method instead."
    )]] constexpr
    operator Underlying() const noexcept {  // NOLINT
        return mask_;
    }

    /// Get the bitmask as the underlying type (integer).
    constexpr Underlying get() const noexcept {
        return mask_;
    }

    /// Set all bits.
    constexpr void set() noexcept {
        mask_ = ~empty;
    }

    /// Set specified bits.
    constexpr void set(T mask) noexcept {
        mask_ |= static_cast<Underlying>(mask);
    }

    /// Reset all bits.
    constexpr void reset() noexcept {
        mask_ = empty;
    }

    /// Reset specified bits.
    constexpr void reset(T mask) noexcept {
        mask_ &= ~static_cast<Underlying>(mask);
    }

    /// Flip all bits.
    constexpr void flip() noexcept {
        mask_ = ~mask_;
    }

    /// Check if all bits are set.
    constexpr bool all() const noexcept {
        return Underlying(~mask_) == empty;
    }

    /// Check if all of the specified bits are set.
    constexpr bool allOf(T mask) const noexcept {
        return (mask_ & static_cast<Underlying>(mask)) == static_cast<Underlying>(mask);
    }

    /// Check if any bits are set.
    constexpr bool any() const noexcept {
        return mask_ != empty;
    }

    /// Check if any of the specified bits are set.
    constexpr bool anyOf(T mask) const noexcept {
        return (mask_ & static_cast<Underlying>(mask)) != empty;
    }

    /// Check if none bits are set.
    constexpr bool none() const noexcept {
        return mask_ == empty;
    }

    /// Check if none of the specified bits are set.
    constexpr bool noneOf(T mask) const noexcept {
        return (mask_ & static_cast<Underlying>(mask)) == empty;
    }

private:
    static constexpr Underlying empty{};
    Underlying mask_{};
};

/// @relates Bitmask
template <typename T, typename U>
constexpr bool operator==(Bitmask<T> lhs, U rhs) noexcept {
    return lhs.get() == Bitmask<T>(rhs).get();
}

/// @relates Bitmask
template <typename T, typename U>
constexpr bool operator!=(Bitmask<T> lhs, U rhs) noexcept {
    return lhs.get() != Bitmask<T>(rhs).get();
}

/// @relates Bitmask
template <typename T, typename U>
constexpr bool operator==(U lhs, Bitmask<T> rhs) noexcept {
    return Bitmask<T>(lhs).get() == rhs.get();
}

/// @relates Bitmask
template <typename T, typename U>
constexpr bool operator!=(U lhs, Bitmask<T> rhs) noexcept {
    return Bitmask<T>(lhs).get() != rhs.get();
}

/**
 * @}
 */

}  // namespace opcua
