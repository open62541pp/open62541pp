#pragma once

#include <iterator>  // distance
#include <string>
#include <type_traits>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Helper.h"
#include "open62541pp/Traits.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/Types.h"
#include "open62541pp/open62541.h"

namespace opcua {

template <Type... T>
struct TypeList {
    using Types = std::integer_sequence<Type, T...>;

    static constexpr size_t size() {
        return sizeof...(T);
    }

    static constexpr bool contains(Type type) {
        return ((type == T) || ...);
    }

    static constexpr auto toArray() {
        return std::array<Type, sizeof...(T)>{T...};
    }
};

template <typename T, typename Enable = void>
struct TypeConverter {
    static_assert(detail::AlwaysFalse<T>::value, "Missing specialization of TypeConverter");

    using ValueType = T;
    using NativeType = std::nullptr_t;
    using ValidTypes = TypeList<>;

    static void fromNative(const NativeType& src, ValueType& dst);
    static void toNative(const ValueType& src, NativeType& dst);
};

/* ------------------------------------------- Helper ------------------------------------------- */

namespace detail {

template <typename T, Type type>
constexpr bool isValidTypeCombination() {
    return TypeConverter<T>::ValidTypes::contains(type);
}

template <typename T>
constexpr bool isValidTypeCombination(Type type) {
    return TypeConverter<T>::ValidTypes::contains(type);
}

template <typename T, Type type>
static constexpr void assertTypeCombination() {
    static_assert(
        detail::isValidTypeCombination<T, type>(), "Invalid template type / type enum combination"
    );
}

template <typename T>
constexpr Type guessType() {
    static_assert(
        TypeConverter<T>::ValidTypes::size() == 1,
        "Ambigious template type, please specify type enum manually"
    );
    return TypeConverter<T>::ValidTypes::toArray().at(0);
}

template <typename It>
constexpr Type guessTypeFromIterator() {
    using ValueType = typename std::iterator_traits<It>::value_type;
    return guessType<ValueType>();
}

/* ------------------------------------- Converter functions ------------------------------------ */

/// Convert and copy from native type.
template <typename T, typename NativeType = typename TypeConverter<T>::NativeType>
T fromNative(NativeType* value) {
    T result{};
    TypeConverter<T>::fromNative(*value, result);
    return result;
}

/// Convert and copy from native type.
/// @warning Type erased version, use with caution.
template <typename T, typename NativeType = typename TypeConverter<T>::NativeType>
T fromNative(void* value, Type type) {
    assert(TypeConverter<T>::ValidTypes::contains(type));  // NOLINT
    return fromNative<T>(static_cast<NativeType*>(value));
}

/// Create and convert vector from native array.
template <typename T, typename NativeType = typename TypeConverter<T>::NativeType>
std::vector<T> fromNativeArray(NativeType* array, size_t size) {
    if constexpr (detail::isNativeType<T>() && std::is_fundamental_v<T>) {
        return std::vector<T>(array, array + size);  // NOLINT
    } else {
        std::vector<T> result(size);
        for (size_t i = 0; i < size; ++i) {
            TypeConverter<T>::fromNative(array[i], result[i]);  // NOLINT
        }
        return result;
    }
}

/// Create and convert vector from native array.
/// @warning Type erased version, use with caution.
template <typename T, typename NativeType = typename TypeConverter<T>::NativeType>
std::vector<T> fromNativeArray(void* array, size_t size, Type type) {
    assert(TypeConverter<T>::ValidTypes::contains(type));  // NOLINT
    return fromNativeArray<T>(static_cast<NativeType*>(array), size);
}

/// Allocate native type.
template <typename TNative, Type type>
TNative* allocNative() {
    assertTypeCombination<TNative, type>();
    const auto* dataType = detail::getUaDataType(type);
    assert(sizeof(TNative) == dataType->memSize);  // NOLINT
    auto* result = static_cast<TNative*>(UA_new(dataType));
    if (result == nullptr) {
        throw std::bad_alloc();
    }
    return result;
}

/// Allocate and copy to native type.
template <typename T, Type type>
auto* toNativeAlloc(const T& value) {
    assertTypeCombination<T, type>();
    using NativeType = typename TypeConverter<T>::NativeType;
    auto* result = allocNative<NativeType, type>();
    TypeConverter<T>::toNative(value, *result);
    return result;
}

/// Allocate native array
template <typename TNative, Type type>
auto* allocNativeArray(size_t size) {
    assertTypeCombination<TNative, type>();
    const auto* dataType = detail::getUaDataType(type);
    assert(sizeof(TNative) == dataType->memSize);  // NOLINT
    auto* result = static_cast<TNative*>(UA_Array_new(size, dataType));
    if (result == nullptr) {
        throw std::bad_alloc();
    }
    return result;
}

/// Allocate and copy iterator range to native array.
template <typename InputIt, Type type>
auto* toNativeArrayAlloc(InputIt first, InputIt last) {
    using ValueType = typename std::iterator_traits<InputIt>::value_type;
    using NativeType = typename TypeConverter<ValueType>::NativeType;
    assertTypeCombination<ValueType, type>();
    const size_t size = std::distance(first, last);
    auto* result = allocNativeArray<NativeType, type>(size);
    for (size_t i = 0; i < size; ++i) {
        TypeConverter<ValueType>::toNative(*first++, result[i]);  // NOLINT
    }
    return result;
}

/// Allocate and copy to native array.
template <typename T, Type type>
auto* toNativeArrayAlloc(const T* array, size_t size) {
    return toNativeArrayAlloc<const T*, type>(array, array + size);  // NOLINT
}

}  // namespace detail

/* ---------------------------- Implementation for native data types ---------------------------- */

namespace detail {

template <typename T, Type... Types>
struct TypeConverterNative {
    using ValueType = T;
    using NativeType = T;
    using ValidTypes = TypeList<Types...>;

    static_assert(ValidTypes::size() >= 1);

    static void fromNative(const T& src, T& dst) {
        if constexpr (std::is_fundamental_v<T>) {
            dst = src;  // shallow copy
        } else {
            // just take first type -> underlying memory layout of all types should be the same
            constexpr auto typeGuess = ValidTypes::toArray().at(0);
            // clear first
            UA_clear(&dst, detail::getUaDataType(typeGuess));
            // deep copy
            const auto status = UA_copy(&src, &dst, detail::getUaDataType(typeGuess));
            detail::checkStatusCodeException(status);
        }
    }

    static void toNative(const T& src, T& dst) {
        fromNative(src, dst);
    }
};

}  // namespace detail

template <>
struct TypeConverter<UA_Boolean> : detail::TypeConverterNative<UA_Boolean, Type::Boolean> {};

template <>
struct TypeConverter<UA_SByte> : detail::TypeConverterNative<UA_SByte, Type::SByte> {};

template <>
struct TypeConverter<UA_Byte> : detail::TypeConverterNative<UA_Byte, Type::Byte> {};

template <>
struct TypeConverter<UA_Int16> : detail::TypeConverterNative<UA_Int16, Type::Int16> {};

template <>
struct TypeConverter<UA_UInt16> : detail::TypeConverterNative<UA_UInt16, Type::UInt16> {};

template <>
struct TypeConverter<UA_Int32> : detail::TypeConverterNative<UA_Int32, Type::Int32> {};

template <>
struct TypeConverter<UA_UInt32> : detail::TypeConverterNative<UA_UInt32, Type::UInt32> {};

template <>
struct TypeConverter<UA_Int64> : detail::TypeConverterNative<UA_Int64, Type::Int64> {};

template <>
struct TypeConverter<UA_UInt64> : detail::TypeConverterNative<UA_UInt64, Type::UInt64> {};

template <>
struct TypeConverter<UA_Float> : detail::TypeConverterNative<UA_Float, Type::Float> {};

template <>
struct TypeConverter<UA_Double> : detail::TypeConverterNative<UA_Double, Type::Double> {};

static_assert(std::is_same_v<UA_String, UA_ByteString>);
static_assert(std::is_same_v<UA_String, UA_XmlElement>);

template <>
struct TypeConverter<UA_String>
    : detail::TypeConverterNative<UA_String, Type::String, Type::ByteString, Type::XmlElement> {};

template <>
struct TypeConverter<UA_Guid> : detail::TypeConverterNative<UA_Guid, Type::Guid> {};

template <>
struct TypeConverter<UA_NodeId> : detail::TypeConverterNative<UA_NodeId, Type::NodeId> {};

template <>
struct TypeConverter<UA_ExpandedNodeId>
    : detail::TypeConverterNative<UA_ExpandedNodeId, Type::ExpandedNodeId> {};

template <>
struct TypeConverter<UA_QualifiedName>
    : detail::TypeConverterNative<UA_QualifiedName, Type::QualifiedName> {};

template <>
struct TypeConverter<UA_LocalizedText>
    : detail::TypeConverterNative<UA_LocalizedText, Type::LocalizedText> {};

template <>
struct TypeConverter<UA_ExtensionObject>
    : detail::TypeConverterNative<UA_ExtensionObject, Type::ExtensionObject> {};

/* ------------------------------- Implementation for TypeWrapper ------------------------------- */

template <typename WrapperType>
struct TypeConverter<WrapperType, std::enable_if_t<detail::IsTypeWrapper<WrapperType>::value>> {
    using NativeConverter = TypeConverter<typename WrapperType::UaType>;

    using ValueType = WrapperType;
    using NativeType = typename WrapperType::UaType;
    using ValidTypes = TypeList<WrapperType::getType()>;

    static void fromNative(const NativeType& src, ValueType& dst) {
        dst = WrapperType(src);
    }

    static void toNative(const ValueType& src, NativeType& dst) {
        NativeConverter::toNative(dst, src.handle());
    }
};

/* ---------------------------- Implementations for std library types --------------------------- */

template <>
struct TypeConverter<std::string> {
    using ValueType = std::string;
    using NativeType = UA_String;
    using ValidTypes = TypeList<Type::String, Type::ByteString, Type::XmlElement>;

    static void fromNative(const NativeType& src, ValueType& dst) {
        dst = detail::toString(src);
    }

    static void toNative(const ValueType& src, NativeType& dst) {
        UA_clear(&dst, detail::getUaDataType(Type::String));
        dst = detail::allocUaString(src);
    }
};

}  // namespace opcua
