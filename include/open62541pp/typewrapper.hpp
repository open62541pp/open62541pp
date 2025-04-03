#pragma once

#include <type_traits>
#include <utility>  // exchange, swap

#include "open62541pp/common.hpp"  // TypeIndex
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/detail/types_handling.hpp"
#include "open62541pp/typeregistry.hpp"
#include "open62541pp/wrapper.hpp"

namespace opcua {

/**
 * Template base class to wrap `UA_*` type objects that require manual memory management.
 *
 * Provides a zero-cost abstraction to wrap C API objects and automatically delete them upon
 * destruction.
 *
 * @warning No virtual constructor is defined; do not implement a destructor in derived classes.
 * @ingroup Wrapper
 */
template <typename T, TypeIndex Index>
class TypeWrapper : public Wrapper<T> {
public:
    static_assert(Index < UA_TYPES_COUNT);

    constexpr TypeWrapper() noexcept = default;

    /// Constructor with native object (deep copy).
    explicit constexpr TypeWrapper(const T& native)
        : Wrapper<T>(detail::copy(native, UA_TYPES[Index])) {}

    /// Constructor with native object (move rvalue).
    constexpr TypeWrapper(T&& native) noexcept  // NOLINT
        : Wrapper<T>(std::exchange(native, {})) {}

    ~TypeWrapper() {
        clear();
    };

    /// Copy constructor (deep copy).
    constexpr TypeWrapper(const TypeWrapper& other)
        : Wrapper<T>(detail::copy(other.native(), UA_TYPES[Index])) {}

    /// Move constructor.
    constexpr TypeWrapper(TypeWrapper&& other) noexcept
        : Wrapper<T>(std::exchange(other.native(), {})) {}

    /// Copy assignment (deep copy).
    constexpr TypeWrapper& operator=(const TypeWrapper& other) {
        if (this != &other) {
            clear();
            this->native() = detail::copy(other.native(), UA_TYPES[Index]);
        }
        return *this;
    }

    /// Copy assignment with native object (deep copy).
    constexpr TypeWrapper& operator=(const T& native) {
        if (&this->native() != &native) {
            clear();
            this->native() = detail::copy(native, UA_TYPES[Index]);
        }
        return *this;
    }

    /// Move assignment.
    constexpr TypeWrapper& operator=(TypeWrapper&& other) noexcept {
        if (this != &other) {
            clear();
            this->native() = std::exchange(other.native(), {});
        }
        return *this;
    }

    /// Move assignment with native object.
    constexpr TypeWrapper& operator=(T&& native) noexcept {  // NOLINT
        if (&this->native() != &native) {
            clear();
            this->native() = std::exchange(native, {});
        }
        return *this;
    }

    /// Get type as type index of the ::UA_TYPES array.
    static constexpr TypeIndex typeIndex() {
        return Index;
    }

protected:
    constexpr void clear() noexcept {
        detail::clear(this->native(), UA_TYPES[Index]);
    }
};

/* -------------------------------------------- Trait ------------------------------------------- */

namespace detail {

template <typename T>
struct IsTypeWrapper {
    // https://stackoverflow.com/a/51910887
    template <typename U, TypeIndex Index>
    static std::true_type check(const TypeWrapper<U, Index>&);

    static std::false_type check(...);

    using type = decltype(check(std::declval<T&>()));  // NOLINT, false positive?
    static constexpr bool value = type::value;
};

}  // namespace detail

/* --------------------------------- TypeRegistry specialization -------------------------------- */

template <typename T>
struct TypeRegistry<T, std::enable_if_t<detail::IsTypeWrapper<T>::value>> {
    static const UA_DataType& getDataType() noexcept {
        return UA_TYPES[T::typeIndex()];
    }
};

}  // namespace opcua
