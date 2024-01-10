#pragma once

#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>  // exchange, swap

#include "open62541pp/Common.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/detail/helper.h"
#include "open62541pp/open62541.h"
#include "open62541pp/overloads/comparison.h"

namespace opcua {

/**
 * @defgroup TypeWrapper Wrapper classes of UA_* types
 *
 * Safe wrapper classes for open62541 `UA_*` types to prevent memory leaks.
 * @n
 * All wrapper classes inherit from opcua::TypeWrapper.
 * Native open62541 objects can be accessed using the opcua::TypeWrapper::handle() method.
 */

/**
 * Template base class to wrap `UA_*` type objects.
 *
 * Zero cost abstraction to wrap the C API objects and delete them on destruction. The derived
 * classes should implement specific constructors to convert from other data types.
 *
 * `TypeWrapper<T, typeIndex>` is pointer-interconvertible to `T`. Use asWrapper(NativeType&) or
 * asWrapper(constNativeType&) to cast native object references to wrapper object references.
 *
 * @warning No virtual constructor defined, don't implement a destructor in the derived classes.
 * @ingroup TypeWrapper
 */
template <typename T, TypeIndex typeIndex>
class TypeWrapper {
public:
    static_assert(typeIndex < UA_TYPES_COUNT);

    using TypeWrapperBase = TypeWrapper<T, typeIndex>;
    using NativeType = T;

    TypeWrapper() = default;

    /// Constructor with native object (deep copy).
    explicit TypeWrapper(const T& native)
        : native_(detail::copy(native, UA_TYPES[typeIndex])) {}

    /// Constructor with native object (move rvalue).
    constexpr TypeWrapper(T&& native) noexcept  // NOLINT, implicit wanted
        : native_(std::exchange(native, {})) {}

    ~TypeWrapper() {  // NOLINT
        clear();
    };

    /// Copy constructor (deep copy).
    TypeWrapper(const TypeWrapper& other)
        : native_(detail::copy(other.native_, UA_TYPES[typeIndex])) {}

    /// Move constructor.
    TypeWrapper(TypeWrapper&& other) noexcept
        : native_(std::exchange(other.native_, {})) {}

    /// Copy assignment (deep copy).
    TypeWrapper& operator=(const TypeWrapper& other) {  // NOLINT, false positive
        if (this != &other) {
            clear();
            native_ = detail::copy(other.native_, UA_TYPES[typeIndex]);
        }
        return *this;
    }

    /// Copy assignment with native object (deep copy).
    TypeWrapper& operator=(const T& native) {
        if (&native_ != &native) {
            clear();
            native_ = detail::copy(native, UA_TYPES[typeIndex]);
        }
        return *this;
    }

    /// Move assignment.
    TypeWrapper& operator=(TypeWrapper&& other) noexcept {
        if (this != &other) {
            std::swap(native_, other.native_);
        }
        return *this;
    }

    /// Move assignment with native object.
    TypeWrapper& operator=(T&& native) noexcept {
        if (&native_ != &native) {
            clear();
            native_ = std::exchange(native, {});
        }
        return *this;
    }

    /// Implicit conversion to native object.
    constexpr operator T&() noexcept {  // NOLINT
        return native_;
    }

    /// Implicit conversion to native object.
    constexpr operator const T&() const noexcept {  // NOLINT
        return native_;
    }

    /// Member access to native object.
    constexpr T* operator->() noexcept {
        return &native_;
    }

    /// Member access to native object.
    constexpr const T* operator->() const noexcept {
        return &native_;
    }

    /// Swap with wrapper object.
    void swap(TypeWrapper& other) noexcept {
        static_assert(std::is_swappable_v<T>);
        std::swap(native_, other.native_);
    }

    /// Swap with native object.
    void swap(T& native) noexcept {
        static_assert(std::is_swappable_v<T>);
        std::swap(native_, native);
    }

    /// Get type as type index of the ::UA_TYPES array.
    static constexpr TypeIndex getTypeIndex() {
        return typeIndex;
    }

    /// Return pointer to native object.
    constexpr T* handle() noexcept {
        return &native_;
    }

    /// Return const pointer to native object.
    constexpr const T* handle() const noexcept {
        return &native_;
    };

protected:
    void clear() noexcept {
        detail::clear(native_, UA_TYPES[typeIndex]);
    }

private:
    T native_{};
};

/* -------------------------------------------- Trait ------------------------------------------- */

namespace detail {

template <typename T>
struct IsTypeWrapper {
    // https://stackoverflow.com/a/51910887
    template <typename U, TypeIndex typeIndex>
    static std::true_type check(const TypeWrapper<U, typeIndex>&);

    static std::false_type check(...);

    using type = decltype(check(std::declval<T&>()));  // NOLINT
    static constexpr bool value = type::value;
};

template <typename T>
inline constexpr bool isTypeWrapper = IsTypeWrapper<T>::value;

}  // namespace detail

/* ------------------------------ Cast native type to wrapper type ------------------------------ */

namespace detail {

template <typename T1, typename T2>
constexpr void assertIsPointerInterconvertible() noexcept {
    static_assert(std::is_standard_layout_v<T1>);
    static_assert(std::is_standard_layout_v<T2>);
    static_assert(sizeof(T1) == sizeof(T2));
}

template <typename WrapperType, typename NativeType>
constexpr void assertIsWrappedType() noexcept {
    static_assert(std::is_same_v<NativeType, typename WrapperType::NativeType>);
}

}  // namespace detail

/**
 * Cast native `UA_*` type object pointers to TypeWrapper object pointers.
 * This is especially helpful to avoid copies in getter methods of composed types.
 * @see https://en.cppreference.com/w/cpp/language/static_cast#pointer-interconvertible
 * @see https://github.com/open62541pp/open62541pp/issues/30
 * @ingroup TypeWrapper
 */
template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
constexpr WrapperType* asWrapper(NativeType* native) noexcept {
    detail::assertIsWrappedType<WrapperType, NativeType>();
    detail::assertIsPointerInterconvertible<WrapperType, NativeType>();
    return static_cast<WrapperType*>(static_cast<void*>(native));
}

/// @copydoc asWrapper(NativeType*)
/// @ingroup TypeWrapper
template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
constexpr const WrapperType* asWrapper(const NativeType* native) noexcept {
    detail::assertIsWrappedType<WrapperType, NativeType>();
    detail::assertIsPointerInterconvertible<WrapperType, NativeType>();
    return static_cast<const WrapperType*>(static_cast<const void*>(native));
}

/**
 * Cast native `UA_*` type object references to TypeWrapper object references.
 * @copydetails asWrapper(NativeType*)
 * @ingroup TypeWrapper
 */
template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
constexpr WrapperType& asWrapper(NativeType& native) noexcept {
    return *asWrapper<WrapperType, NativeType>(&native);
}

/// @copydoc asWrapper(NativeType&)
/// @ingroup TypeWrapper
template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
constexpr const WrapperType& asWrapper(const NativeType& native) noexcept {
    return *asWrapper<WrapperType, NativeType>(&native);
}

/**
 * Cast TypeWrapper object pointers to native `UA_*` type object pointers.
 * @see https://en.cppreference.com/w/cpp/language/static_cast#pointer-interconvertible
 * @ingroup TypeWrapper
 */
template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
constexpr NativeType* asNative(WrapperType* wrapper) noexcept {
    detail::assertIsWrappedType<WrapperType, NativeType>();
    detail::assertIsPointerInterconvertible<WrapperType, NativeType>();
    return static_cast<NativeType*>(static_cast<void*>(wrapper));
}

/// @copydoc asNative(WrapperType*)
/// @ingroup TypeWrapper
template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
constexpr const NativeType* asNative(const WrapperType* wrapper) noexcept {
    detail::assertIsWrappedType<WrapperType, NativeType>();
    detail::assertIsPointerInterconvertible<WrapperType, NativeType>();
    return static_cast<const NativeType*>(static_cast<const void*>(wrapper));
}

/**
 * Cast TypeWrapper object references to native `UA_*` type object references.
 * @copydetails asNative(WrapperType*)
 * @ingroup TypeWrapper
 */
template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
constexpr NativeType& asNative(WrapperType& wrapper) noexcept {
    return *asNative<WrapperType, NativeType>(&wrapper);
}

/// @copydoc asNative(WrapperType&)
/// @ingroup TypeWrapper
template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
constexpr const NativeType& asNative(const WrapperType& wrapper) noexcept {
    return *asNative<WrapperType, NativeType>(&wrapper);
}

/* ----------------------------------------- Comparison ----------------------------------------- */

// generate from UA_* type comparison

template <typename T, typename = std::enable_if_t<detail::isTypeWrapper<T>>>
inline bool operator==(const T& lhs, const T& rhs) noexcept {
    return (*lhs.handle() == *rhs.handle());
}

template <typename T, typename = std::enable_if_t<detail::isTypeWrapper<T>>>
inline bool operator!=(const T& lhs, const T& rhs) noexcept {
    return (*lhs.handle() != *rhs.handle());
}

template <typename T, typename = std::enable_if_t<detail::isTypeWrapper<T>>>
inline bool operator<(const T& lhs, const T& rhs) noexcept {
    return (*lhs.handle() < *rhs.handle());
}

template <typename T, typename = std::enable_if_t<detail::isTypeWrapper<T>>>
inline bool operator>(const T& lhs, const T& rhs) noexcept {
    return (*lhs.handle() > *rhs.handle());
}

template <typename T, typename = std::enable_if_t<detail::isTypeWrapper<T>>>
inline bool operator<=(const T& lhs, const T& rhs) noexcept {
    return (*lhs.handle() <= *rhs.handle());
}

template <typename T, typename = std::enable_if_t<detail::isTypeWrapper<T>>>
inline bool operator>=(const T& lhs, const T& rhs) noexcept {
    return (*lhs.handle() >= *rhs.handle());
}

}  // namespace opcua
