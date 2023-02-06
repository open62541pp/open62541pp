#pragma once

#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>  // move, swap

#include "open62541pp/Common.h"
#include "open62541pp/Comparison.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Helper.h"
#include "open62541pp/open62541.h"

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
 * Template base class to wrap UA_* type objects.
 *
 * The derived classes should implement specific constructors to convert from other data types.
 * @ingroup TypeWrapper
 */
template <typename T, uint16_t typeIndex>
class TypeWrapper {
public:
    static_assert(typeIndex < UA_TYPES_COUNT);

    using TypeWrapperBase = TypeWrapper<T, typeIndex>;
    using UaType = T;

    TypeWrapper() = default;

    /// Constructor with native UA_* type (deep copy).
    explicit TypeWrapper(const T& data) {
        copy(data);
    }

    /// Constructor with native UA_* type (move rvalue).
    constexpr explicit TypeWrapper(T&& data) noexcept
        : data_(data) {}

    virtual ~TypeWrapper() {
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

    /// Copy assignment with UA_* type (deep copy).
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

    /// Move assignment with UA_* type.
    TypeWrapper& operator=(T&& other) noexcept {
        clear();
        data_ = other;
        return *this;
    }

    /// Implicit conversion to wrapped object.
    constexpr operator T&() noexcept {  // NOLINT
        return data_;
    }

    /// Implicit conversion to wrapped object.
    constexpr operator const T&() const noexcept {  // NOLINT
        return data_;
    }

    /// Member access to wrapped object.
    constexpr T* operator->() noexcept {
        return &data_;
    }

    /// Member access to wrapped object.
    constexpr const T* operator->() const noexcept {
        return &data_;
    }

    /// Swap wrapped objects.
    void swap(TypeWrapper& other) noexcept {
        static_assert(std::is_swappable_v<T>);
        std::swap(this->data_, other.data_);
    }

    /// Get type as type index.
    static constexpr uint16_t getTypeIndex() {
        return typeIndex;
    }

    /// Get type as Type enum (only for builtin types).
    static constexpr Type getType() {
        static_assert(typeIndex < UA_BUILTIN_TYPES_COUNT, "Only possible for builtin types");
        return static_cast<Type>(typeIndex);
    }

    /// Get type as UA_DataType object.
    static const UA_DataType* getDataType() {
        return detail::getUaDataType<typeIndex>();
    }

    /// Return pointer to wrapped object.
    constexpr T* handle() noexcept {
        return &data_;
    }

    /// Return const pointer to wrapped object.
    constexpr const T* handle() const noexcept {
        return &data_;
    };

protected:
    inline static void checkMemSize() {
        assert(sizeof(T) == getDataType()->memSize);  // NOLINT
    }

    void clear() noexcept {
        checkMemSize();
        UA_clear(&data_, getDataType());
    }

    void copy(const T& data) {
        clear();
        checkMemSize();
        auto status = UA_copy(&data, &data_, getDataType());  // deep copy of data
        detail::throwOnBadStatus(status);
    }

private:
    T data_{};
};

/* -------------------------------------------- Trait ------------------------------------------- */

namespace detail {

// https://stackoverflow.com/a/51910887
template <typename T, uint16_t typeIndex>
std::true_type isTypeWrapperImpl(TypeWrapper<T, typeIndex>*);
std::false_type isTypeWrapperImpl(...);

template <typename T>
using IsTypeWrapper = decltype(isTypeWrapperImpl(std::declval<T*>()));

}  // namespace detail

/* ----------------------------------------- Comparison ----------------------------------------- */

// generate from UA_* type comparison

template <typename T, typename = std::enable_if_t<detail::IsTypeWrapper<T>::value>>
inline bool operator==(const T& left, const T& right) noexcept {
    return (*left.handle() == *right.handle());
}

template <typename T, typename = std::enable_if_t<detail::IsTypeWrapper<T>::value>>
inline bool operator!=(const T& left, const T& right) noexcept {
    return (*left.handle() != *right.handle());
}

template <typename T, typename = std::enable_if_t<detail::IsTypeWrapper<T>::value>>
inline bool operator<(const T& left, const T& right) noexcept {
    return (*left.handle() < *right.handle());
}

template <typename T, typename = std::enable_if_t<detail::IsTypeWrapper<T>::value>>
inline bool operator>(const T& left, const T& right) noexcept {
    return (*left.handle() > *right.handle());
}

template <typename T, typename = std::enable_if_t<detail::IsTypeWrapper<T>::value>>
inline bool operator<=(const T& left, const T& right) noexcept {
    return (*left.handle() <= *right.handle());
}

template <typename T, typename = std::enable_if_t<detail::IsTypeWrapper<T>::value>>
inline bool operator>=(const T& left, const T& right) noexcept {
    return (*left.handle() >= *right.handle());
}

}  // namespace opcua
