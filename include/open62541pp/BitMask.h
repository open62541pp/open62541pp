#pragma once

#include <type_traits>

namespace opcua {

/**
 * Trait to define an enum (class) as a bit mask and allow bitwise operations.
 *
 * @code{.cpp}
 * // define enum (class)
 * enum class Access {
 *     Read  = 0x01U << 0U,
 *     Write = 0x01U << 1U,
 * };
 *
 * // allow bitwise operations
 * template <>
 * struct IsBitMaskEnum<Access> : std::true_type {};
 *
 * // use bitwise operations
 * Access mask = Access::Read | Access::Write;
 * @endcode
 */
template <typename T>
struct IsBitMaskEnum : std::false_type {};

/* -------------------------------------- Bitwise operators ------------------------------------- */

template <typename T>
constexpr typename std::enable_if_t<IsBitMaskEnum<T>::value, T> operator&(T lhs, T rhs) noexcept {
    using U = typename std::underlying_type_t<T>;
    return static_cast<T>(static_cast<U>(lhs) & static_cast<U>(rhs));
}

template <typename T>
constexpr typename std::enable_if_t<IsBitMaskEnum<T>::value, T> operator|(T lhs, T rhs) noexcept {
    using U = typename std::underlying_type_t<T>;
    return static_cast<T>(static_cast<U>(lhs) | static_cast<U>(rhs));
}

template <typename T>
constexpr typename std::enable_if_t<IsBitMaskEnum<T>::value, T> operator^(T lhs, T rhs) noexcept {
    using U = typename std::underlying_type_t<T>;
    return static_cast<T>(static_cast<U>(lhs) ^ static_cast<U>(rhs));
}

template <typename T>
constexpr typename std::enable_if_t<IsBitMaskEnum<T>::value, T> operator~(T rhs) noexcept {
    using U = typename std::underlying_type_t<T>;
    return static_cast<T>(~static_cast<U>(rhs));
}

/* -------------------------------- Bitwise assignment operators -------------------------------- */

template <typename T>
constexpr typename std::enable_if_t<IsBitMaskEnum<T>::value, T> operator|=(T& lhs, T rhs) noexcept {
    using U = typename std::underlying_type_t<T>;
    lhs = static_cast<T>(static_cast<U>(lhs) | static_cast<U>(rhs));
    return lhs;
}

template <typename T>
constexpr typename std::enable_if_t<IsBitMaskEnum<T>::value, T> operator&=(T& lhs, T rhs) noexcept {
    using U = typename std::underlying_type_t<T>;
    lhs = static_cast<T>(static_cast<U>(lhs) & static_cast<U>(rhs));
    return lhs;
}

template <typename T>
constexpr typename std::enable_if_t<IsBitMaskEnum<T>::value, T> operator^=(T& lhs, T rhs) noexcept {
    using U = typename std::underlying_type_t<T>;
    lhs = static_cast<T>(static_cast<U>(lhs) ^ static_cast<U>(rhs));
    return lhs;
}

/* --------------------------------------- Bit mask class --------------------------------------- */

/**
 * Bit mask using (scoped) enums.
 * Zero-cost abstraction to specify bit masks with enums/ints or enum classes.
 *
 * @code{.cpp}
 * // construct with scoped enums
 * BitMask<NodeClass> mask = NodeClass::Variable | NodeClass::Object;
 * // construct with unscoped enums or ints
 * BitMask<NodeClass> mask = UA_NODECLASS_VARIABLE | UA_NODECLASS_OBJECT;
 * @endcode
 *
 * @tparam T Enumeration type
 *
 * @see https://www.strikerx3.dev/cpp/2019/02/27/typesafe-enum-class-bitmasks-in-cpp.html
 * @see https://andreasfertig.blog/2024/01/cpp20-concepts-applied/
 */
template <typename T>
class BitMask {
public:
    static_assert(std::is_enum_v<T>);
    static_assert(IsBitMaskEnum<T>::value);

    using Underlying = std::underlying_type_t<T>;

    /// Create an empty bit mask.
    constexpr BitMask() noexcept = default;

    /// Create a bit mask from the enumeration type.
    constexpr BitMask(T value) noexcept  // NOLINT, implicit wanted
        : mask_(static_cast<Underlying>(value)) {}

    /// Create a bit mask from the underlying type.
    constexpr BitMask(Underlying value) noexcept  // NOLINT, implicit wanted
        : mask_(value) {}

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

private:
    Underlying mask_;
};

}  // namespace opcua
