#pragma once

#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>  // move, swap

#include "open62541pp/Common.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/detail/helper.h"
#include "open62541pp/open62541.h"
#include "open62541pp/overloads/comparison.h"

namespace opcua {

/**
 * @defgroup TypeWrapper Wrapper classes of UA_* types
 *
 * Safe wrapper classes for heap-allocated open62541 `UA_*` types to prevent memory leaks.
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
    explicit TypeWrapper(const T& data) {
        copy(data);
    }

    /// Constructor with native object (move rvalue).
    constexpr TypeWrapper(T&& data) noexcept  // NOLINT, implicit wanted
        : data_(data) {}

    ~TypeWrapper() {  // NOLINT
        clear();
    };

    /// Copy constructor (deep copy).
    TypeWrapper(const TypeWrapper& other) {
        copy(other.data_);
    }

    /// Move constructor.
    TypeWrapper(TypeWrapper&& other) noexcept {
        swap(other);
    }

    /// Copy assignment (deep copy).
    TypeWrapper& operator=(const TypeWrapper& other) {  // NOLINT, false positive
        if (this == &other) {
            return *this;
        }
        copy(other.data_);
        return *this;
    }

    /// Copy assignment with native object (deep copy).
    TypeWrapper& operator=(const T& other) {
        copy(other);
        return *this;
    }

    /// Move assignment.
    TypeWrapper& operator=(TypeWrapper&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        swap(other);
        return *this;
    }

    /// Move assignment with native object.
    TypeWrapper& operator=(T&& other) noexcept {
        clear();
        data_ = other;
        return *this;
    }

    /// Implicit conversion to native object.
    constexpr operator T&() noexcept {  // NOLINT
        return data_;
    }

    /// Implicit conversion to native object.
    constexpr operator const T&() const noexcept {  // NOLINT
        return data_;
    }

    /// Member access to native object.
    constexpr T* operator->() noexcept {
        return &data_;
    }

    /// Member access to native object.
    constexpr const T* operator->() const noexcept {
        return &data_;
    }

    /// Swap with wrapper object.
    void swap(TypeWrapper& other) noexcept {
        static_assert(std::is_swappable_v<T>);
        std::swap(data_, other.data_);
    }

    /// Swap with native object.
    void swap(T& other) noexcept {
        static_assert(std::is_swappable_v<T>);
        std::swap(data_, other);
    }

    /// Get type as type index of the ::UA_TYPES array.
    static constexpr TypeIndex getTypeIndex() {
        return typeIndex;
    }

    /// Return pointer to native object.
    constexpr T* handle() noexcept {
        return &data_;
    }

    /// Return const pointer to native object.
    constexpr const T* handle() const noexcept {
        return &data_;
    };

protected:
    inline static void checkMemSize() {
        assert(sizeof(T) == UA_TYPES[typeIndex].memSize);
    }

    void clear() noexcept {
        checkMemSize();
        UA_clear(&data_, &UA_TYPES[typeIndex]);
    }

    void copy(const T& data) {
        clear();
        checkMemSize();
        auto status = UA_copy(&data, &data_, &UA_TYPES[typeIndex]);  // deep copy of data
        detail::throwOnBadStatus(status);
    }

private:
    T data_{};
};

/* -------------------------------------------- Trait ------------------------------------------- */

namespace detail {

// https://stackoverflow.com/a/51910887
template <typename T, TypeIndex typeIndex>
std::true_type isTypeWrapperImpl(TypeWrapper<T, typeIndex>*);
std::false_type isTypeWrapperImpl(...);

template <typename T>
using IsTypeWrapper = decltype(isTypeWrapperImpl(std::declval<T*>()));

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

template <typename T, typename = std::enable_if_t<detail::IsTypeWrapper<T>::value>>
inline bool operator==(const T& lhs, const T& rhs) noexcept {
    return (*lhs.handle() == *rhs.handle());
}

template <typename T, typename = std::enable_if_t<detail::IsTypeWrapper<T>::value>>
inline bool operator!=(const T& lhs, const T& rhs) noexcept {
    return (*lhs.handle() != *rhs.handle());
}

template <typename T, typename = std::enable_if_t<detail::IsTypeWrapper<T>::value>>
inline bool operator<(const T& lhs, const T& rhs) noexcept {
    return (*lhs.handle() < *rhs.handle());
}

template <typename T, typename = std::enable_if_t<detail::IsTypeWrapper<T>::value>>
inline bool operator>(const T& lhs, const T& rhs) noexcept {
    return (*lhs.handle() > *rhs.handle());
}

template <typename T, typename = std::enable_if_t<detail::IsTypeWrapper<T>::value>>
inline bool operator<=(const T& lhs, const T& rhs) noexcept {
    return (*lhs.handle() <= *rhs.handle());
}

template <typename T, typename = std::enable_if_t<detail::IsTypeWrapper<T>::value>>
inline bool operator>=(const T& lhs, const T& rhs) noexcept {
    return (*lhs.handle() >= *rhs.handle());
}

}  // namespace opcua
