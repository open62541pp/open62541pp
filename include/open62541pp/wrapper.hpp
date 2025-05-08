#pragma once

#include <type_traits>
#include <utility>  // exchange, move

#include "open62541pp/common.hpp"  // TypeIndex
#include "open62541pp/detail/types_handling.hpp"
#include "open62541pp/typeregistry.hpp"

namespace opcua {

/**
 * @defgroup Wrapper Wrapper classes
 *
 * All wrapper classes inherit from Wrapper.
 * Native open62541 objects can be accessed using the Wrapper::handle() member function.
 *
 * Wrapper types are pointer-interconvertible to the wrapped native type and vice versa:
 * - Use asWrapper(NativeType*) or asWrapper(const NativeType*) to cast native object pointers to wrapper object pointers.
 * - Use asWrapper(NativeType&) or asWrapper(const NativeType&) to cast native object references to wrapper object references.
 * - Use asNative(WrapperType*) or asNative(const WrapperType*) to cast wrapper object pointers to native object pointers.
 * - Use asNative(WrapperType&) or asNative(const WrapperType&) to cast wrapper object references to native object references.
 *
 * According to the standard:
 * > Two objects `a` and `b` are pointer-interconvertible if:
 * > One is a standard-layout class object [wrapper] and the other is the first non-static data
 * > member of that object [wrapped native type].
 * Derived classes must fulfill the requirements of standard-layout types to be convertible.
 * @see https://en.cppreference.com/w/cpp/language/static_cast#pointer-interconvertible
 */

/**
 * Default type handler providing standard copy and move operations.
 *
 * You can specialize it for custom types to override default behavior. For example:
 * @code
 * template <>
 * struct TypeHandler<MyType> {
 *     static constexpr MyType copy(const MyType& obj) { ... }
 *     static constexpr MyType move(MyType&& obj) noexcept { ... }
 *     static constexpr void clear(MyType& obj) noexcept { ... }
 * };
 * @endcode
 *
 * @ingroup Wrapper
 */
template <typename T>
struct TypeHandler {
    static_assert(std::is_copy_constructible_v<T>);
    static_assert(std::is_nothrow_move_constructible_v<T>);
    static_assert(std::is_nothrow_default_constructible_v<T>);

    static constexpr T copy(const T& object) {
        return object;
    };

    static constexpr T move(T&& object) noexcept {
        return std::move(object);
    };
};

/**
 * Specialized type handler for native types.
 *
 * @tparam T Native type, e.g. @ref UA_String
 * @tparam Index Type index of the @ref UA_TYPES array, e.g. `UA_TYPES_STRING`
 *
 * @see TypeHandler
 * @ingroup Wrapper
 */
template <typename T, TypeIndex Index>
struct TypeHandlerNative {
    static_assert(Index < UA_TYPES_COUNT);

    static constexpr T copy(const T& native) {
        return detail::copy(native, UA_TYPES[Index]);
    };

    static constexpr T move(T&& native) noexcept {  // NOLINT(*not-moved)
        return std::exchange(native, {});
    };

    static constexpr void clear(T& native) noexcept {
        detail::clear(native, UA_TYPES[Index]);
    };
};

namespace detail {

template <typename Handler, typename T, typename = void>
struct HasClear : std::false_type {};

template <typename Handler, typename T>
struct HasClear<Handler, T, std::void_t<decltype(Handler::clear(std::declval<T&>()))>>
    : std::true_type {};

template <typename WrapperType, bool Clear>
class WrapperDestructorMixin {};

template <typename WrapperType>
class WrapperDestructorMixin<WrapperType, true> {  // NOLINT(*special-member-functions)
public:
    ~WrapperDestructorMixin() noexcept {
        auto& native = *static_cast<WrapperType*>(this)->handle();
        WrapperType::HandlerType::clear(native);
    }
};

}  // namespace detail

/**
 * Template base class to wrap (native) objects.
 *
 * @tparam T Type of the native object.
 * @tparam Handler Policy type that defines object operations. It must define:
 *         - `static T copy(const T&)` to copy the wrapped object,
 *         - `static T move(T&&) noexcept` to move the wrapped object,
 *         - `static void clear(T&) noexcept` to clear the wrapped object (optional).
 *            If clear is not defined, the wrapper does not define a destructor.
 *
 * The default @ref TypeHandler provides standard copy/move behavior.
 * It can be specialized for custom behavior per type.
 * Use @ref TypeHandlerNative for native open62541 types.
 * @ref WrapperNative is a convenience alias for Wrapper using TypeHandlerNative.
 *
 * @warning No virtual constructor is defined; do not implement a destructor in derived classes.
 * @ingroup Wrapper
 */
template <typename T, typename Handler = TypeHandler<T>>
class Wrapper
    : public detail::WrapperDestructorMixin<
          Wrapper<T, Handler>,
          detail::HasClear<Handler, T>::value> {
public:
    static_assert(std::is_nothrow_default_constructible_v<T>);
    static_assert(std::is_invocable_v<decltype(Handler::copy), const T&>);
    static_assert(std::is_nothrow_invocable_v<decltype(Handler::move), T&&>);

    using NativeType = T;
    using HandlerType = Handler;

    constexpr Wrapper() noexcept = default;

    /// Copy constructor.
    constexpr Wrapper(const Wrapper& other)
        : native_{Handler::copy(other.native())} {}

    /// Copy constructor with native object.
    constexpr explicit Wrapper(const T& native)
        : native_{Handler::copy(native)} {}

    /// Move constructor.
    constexpr Wrapper(Wrapper&& other) noexcept
        : native_{Handler::move(std::move(other.native()))} {}

    /// Move constructor with native object.
    constexpr Wrapper(T&& native) noexcept  // NOLINT(*explicit-conversions)
        : native_{Handler::move(std::move(native))} {}

    ~Wrapper() noexcept = default;

    /// Copy assignment.
    constexpr Wrapper& operator=(const Wrapper& other) {
        if (this != &other) {
            clear();
            this->native() = Handler::copy(other.native());
        }
        return *this;
    }

    /// Copy assignment with native object.
    constexpr Wrapper& operator=(const T& native) {
        if (&this->native() != &native) {
            clear();
            this->native() = Handler::copy(native);
        }
        return *this;
    }

    /// Move assignment.
    constexpr Wrapper& operator=(Wrapper&& other) noexcept {
        if (this != &other) {
            clear();
            this->native() = Handler::move(std::move(other.native()));
        }
        return *this;
    }

    /// Move assignment with native object.
    constexpr Wrapper& operator=(T&& native) noexcept {
        if (&this->native() != &native) {
            clear();
            this->native() = Handler::move(std::move(native));
        }
        return *this;
    }

    /// Implicit conversion to native object.
    constexpr operator T&() noexcept {  // NOLINT(*explicit-conversions)
        return native_;
    }

    /// Implicit conversion to native object.
    constexpr operator const T&() const noexcept {  // NOLINT(*explicit-conversions)
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

    /// Return pointer to native object.
    constexpr T* handle() noexcept {
        return &native_;
    }

    /// Return pointer to native object.
    constexpr const T* handle() const noexcept {
        return &native_;
    }

    /// Swap with wrapper object.
    constexpr void swap(Wrapper& other) noexcept {
        using std::swap;
        swap(this->native(), other.native());
    }

    /// Swap with native object.
    constexpr void swap(T& native) noexcept {
        using std::swap;
        swap(this->native(), native);
    }

protected:
    constexpr const T& native() const noexcept {
        return native_;
    }

    constexpr T& native() noexcept {
        return native_;
    }

    constexpr void clear() noexcept {
        if constexpr (detail::HasClear<Handler, T>::value) {
            Handler::clear(native_);
        } else {
            native_ = {};
        }
    }

private:
    T native_{};
};


/**
 * Convenience alias for Wrapper using TypeHandlerNative.
 * @see Wrapper
 * @ingroup Wrapper
 */
template <typename T, TypeIndex Index>
using WrapperNative = Wrapper<T, TypeHandlerNative<T, Index>>;

template <typename T, TypeIndex Index>
class [[deprecated("use WrapperNative instead")]] TypeWrapper : public WrapperNative<T, Index> {
public:
    using WrapperNative<T, Index>::WrapperNative;
    using WrapperNative<T, Index>::operator=;

    static constexpr TypeIndex typeIndex() {
        return Index;
    }
};

/* -------------------------------------------- Trait ------------------------------------------- */

namespace detail {

template <typename T>
struct IsWrapper {
    // https://stackoverflow.com/a/51910887
    template <typename U, typename Handler>
    static std::true_type check(const Wrapper<U, Handler>&);
    static std::false_type check(...);

    using type = decltype(check(std::declval<T&>()));  // NOLINT
    static constexpr bool value = type::value;
};

}  // namespace detail

/* ------------------------------ Cast native type to wrapper type ------------------------------ */

namespace detail {

template <typename WrapperType>
struct WrapperConversion {
    static_assert(detail::IsWrapper<WrapperType>::value);
    static_assert(std::is_standard_layout_v<WrapperType>);

    using NativeType = typename WrapperType::NativeType;

    // NOLINTBEGIN(bugprone-casting-through-void)
    static constexpr WrapperType* asWrapper(NativeType* native) noexcept {
        return static_cast<WrapperType*>(static_cast<void*>(native));
    }

    static constexpr const WrapperType* asWrapper(const NativeType* native) noexcept {
        return static_cast<const WrapperType*>(static_cast<const void*>(native));
    }

    static constexpr WrapperType& asWrapper(NativeType& native) noexcept {
        return *asWrapper(&native);
    }

    static constexpr const WrapperType& asWrapper(const NativeType& native) noexcept {
        return *asWrapper(&native);
    }

    static constexpr NativeType* asNative(WrapperType* wrapper) noexcept {
        return static_cast<NativeType*>(static_cast<void*>(wrapper));
    }

    static constexpr const NativeType* asNative(const WrapperType* wrapper) noexcept {
        return static_cast<const NativeType*>(static_cast<const void*>(wrapper));
    }

    static constexpr NativeType& asNative(WrapperType& wrapper) noexcept {
        return *asNative(&wrapper);
    }

    static constexpr const NativeType& asNative(const WrapperType& wrapper) noexcept {
        return *asNative(&wrapper);
    }

    // NOLINTEND(bugprone-casting-through-void)
};

}  // namespace detail

/**
 * @ingroup Wrapper
 * @{
 */

/// Cast native object pointers to Wrapper object pointers.
/// This is especially helpful to avoid copies in getter methods of composed types.
/// @see https://github.com/open62541pp/open62541pp/issues/30
/// @relatesalso Wrapper
template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
constexpr WrapperType* asWrapper(NativeType* native) noexcept {
    return detail::WrapperConversion<WrapperType>::asWrapper(native);
}

/// @copydoc asWrapper(NativeType*)
/// @relatesalso Wrapper
template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
constexpr const WrapperType* asWrapper(const NativeType* native) noexcept {
    return detail::WrapperConversion<WrapperType>::asWrapper(native);
}

/// Cast native object references to Wrapper object references.
/// @copydetails asWrapper(NativeType*)
/// @relatesalso Wrapper
template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
constexpr WrapperType& asWrapper(NativeType& native) noexcept {
    return detail::WrapperConversion<WrapperType>::asWrapper(native);
}

/// @copydoc asWrapper(NativeType&)
/// @relatesalso Wrapper
template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
constexpr const WrapperType& asWrapper(const NativeType& native) noexcept {
    return detail::WrapperConversion<WrapperType>::asWrapper(native);
}

/// Cast Wrapper object pointers to native object pointers.
/// @relatesalso Wrapper
template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
constexpr NativeType* asNative(WrapperType* wrapper) noexcept {
    return detail::WrapperConversion<WrapperType>::asNative(wrapper);
}

/// @copydoc asNative(WrapperType*)
/// @relatesalso Wrapper
template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
constexpr const NativeType* asNative(const WrapperType* wrapper) noexcept {
    return detail::WrapperConversion<WrapperType>::asNative(wrapper);
}

/// Cast Wrapper object references to native object references.
/// @copydetails asNative(WrapperType*)
/// @relatesalso Wrapper
template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
constexpr NativeType& asNative(WrapperType& wrapper) noexcept {
    return detail::WrapperConversion<WrapperType>::asNative(wrapper);
}

/// @copydoc asNative(WrapperType&)
/// @relatesalso Wrapper
template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
constexpr const NativeType& asNative(const WrapperType& wrapper) noexcept {
    return detail::WrapperConversion<WrapperType>::asNative(wrapper);
}

/* --------------------------------- TypeRegistry specialization -------------------------------- */

template <typename T>
struct TypeRegistry<
    T,
    std::enable_if_t<
        detail::IsWrapper<T>::value && detail::IsRegistered<typename T::NativeType>::value>> {
    static const UA_DataType& getDataType() noexcept {
        return TypeRegistry<typename T::NativeType>::getDataType();
    }
};

/**
 * @}
 */

}  // namespace opcua
