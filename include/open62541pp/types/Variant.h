#pragma once

#include <cassert>
#include <cstdint>
#include <iterator>  // distance
#include <optional>
#include <utility>  // as_const
#include <vector>

#include "open62541pp/Common.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/TypeConverter.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/helper.h"
#include "open62541pp/open62541.h"

namespace opcua {

// forward declarations
class NodeId;

/**
 * UA_Variant wrapper class.
 * @ingroup TypeWrapper
 */
class Variant : public TypeWrapper<UA_Variant, UA_TYPES_VARIANT> {
public:
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    /// Create Variant from scalar value (no copy if assignable without conversion).
    template <typename T, Type type = detail::guessType<T>()>
    [[nodiscard]] static Variant fromScalar(T& value);

    /// Create Variant from scalar value (copy).
    template <typename T, Type type = detail::guessType<T>()>
    [[nodiscard]] static Variant fromScalar(const T& value);

    /// Create Variant from array (no copy if assignable without conversion).
    template <typename T, Type type = detail::guessType<T>()>
    [[nodiscard]] static Variant fromArray(T* array, size_t size);

    /// Create Variant from array (copy).
    template <typename T, Type type = detail::guessType<T>()>
    [[nodiscard]] static Variant fromArray(const T* array, size_t size);

    /// Create Variant from std::vector (no copy if assignable without conversion).
    template <typename T, Type type = detail::guessType<T>()>
    [[nodiscard]] static Variant fromArray(std::vector<T>& array) {
        return fromArray<T, type>(array.data(), array.size());
    }

    /// Create Variant from std::vector (copy).
    template <typename T, Type type = detail::guessType<T>()>
    [[nodiscard]] static Variant fromArray(const std::vector<T>& array) {
        return fromArray<T, type>(array.data(), array.size());
    }

    /// Create Variant from range of elements (copy).
    template <typename InputIt, Type type = detail::guessTypeFromIterator<InputIt>()>
    [[nodiscard]] static Variant fromArray(InputIt first, InputIt last);

    /// Check if variant is empty.
    bool isEmpty() const noexcept;
    /// Check if variant is a scalar.
    bool isScalar() const noexcept;
    /// Check if variant is an array.
    bool isArray() const noexcept;

    /// Check if variant type is equal to data type.
    bool isType(const UA_DataType* type) const noexcept;
    /// Check if variant type is equal to type enum.
    bool isType(Type type) const noexcept;
    /// Check if variant type is equal to data type node id.
    bool isType(const NodeId& id) const noexcept;

    /// Get data type.
    const UA_DataType* getDataType() const noexcept;

    /// Get variant type.
    std::optional<Type> getVariantType() const noexcept;

    /// Get reference to scalar value with given template type (only native or wrapper types).
    /// @exception BadVariantAccess If the variant is not a scalar or not of type `T`.
    template <typename T>
    T& getScalar();

    /// Get const reference to scalar value with given template type (only native or wrapper types).
    /// @exception BadVariantAccess If the variant is not a scalar or not of type `T`.
    template <typename T>
    const T& getScalar() const;

    /// Get copy of scalar value with given template type.
    /// @exception BadVariantAccess If the variant is not a scalar or not convertible to `T`.
    template <typename T>
    T getScalarCopy() const;

    /// Get array length or 0 if variant is not an array.
    size_t getArrayLength() const noexcept;

    /// Get array dimensions.
    std::vector<uint32_t> getArrayDimensions() const;

    /// Get pointer to array with given template type (only native or wrapper types).
    /// @exception BadVariantAccess If the variant is not an array or not of type `T`.
    template <typename T>
    T* getArray();

    /// Get const pointer to array with given template type (only native or wrapper types).
    /// @exception BadVariantAccess If the variant is not an array or not of type `T`.
    template <typename T>
    const T* getArray() const;

    /// Get copy of array with given template type and return it as a std::vector.
    /// @exception BadVariantAccess If the variant is not an array or not convertible to `T`.
    template <typename T>
    std::vector<T> getArrayCopy() const;

    /// Assign scalar value to variant.
    template <typename T, Type type = detail::guessType<T>()>
    void setScalar(T& value) noexcept;

    /// Assign scalar value to variant with custom data type.
    template <typename T>
    void setScalar(T& value, const UA_DataType& dataType) noexcept;

    /// Copy scalar value to variant.
    template <typename T, Type type = detail::guessType<T>()>
    void setScalarCopy(const T& value);

    /// Copy scalar value to variant with custom data type.
    template <typename T>
    void setScalarCopy(const T& value, const UA_DataType& dataType);

    /// Assign array (raw) to variant.
    template <typename T, Type type = detail::guessType<T>()>
    void setArray(T* array, size_t size) noexcept;

    /// Assign array (std::vector) to variant.
    template <typename T, Type type = detail::guessType<T>()>
    void setArray(std::vector<T>& array) noexcept {
        setArray<T, type>(array.data(), array.size());
    }

    /// Copy range of elements as array to variant.
    template <typename InputIt, Type type = detail::guessTypeFromIterator<InputIt>()>
    void setArrayCopy(InputIt first, InputIt last);

    /// Copy array (raw) to variant.
    template <typename T, Type type = detail::guessType<T>()>
    void setArrayCopy(const T* array, size_t size);

    /// Copy array (std::vector) to variant.
    template <typename T, Type type = detail::guessType<T>()>
    void setArrayCopy(const std::vector<T>& array) {
        setArrayCopy<T, type>(array.data(), array.size());
    }

private:
    template <typename T>
    static constexpr bool isConvertibleToNative() {
        return detail::isNativeType<T>() || detail::IsTypeWrapper<T>::value;
    }

    template <typename T>
    static constexpr void assertGetNoCopy() {
        static_assert(
            isConvertibleToNative<T>(),
            "Template type must be a native or wrapper type to get scalar/array without copy"
        );
    }

    template <typename T>
    static constexpr void assertSetNoCopy() {
        static_assert(
            isConvertibleToNative<T>(),
            "Template type must be a native or wrapper type to assign scalar/array without copy"
        );
    }

    template <Type type>
    static constexpr void assertNoVariant() {
        static_assert(type != Type::Variant, "Variants cannot directly contain another variant");
    }

    void checkIsScalar() const;
    void checkIsArray() const;

    template <typename T>
    inline static void checkDataType([[maybe_unused]] const UA_DataType& dataType) {
        assert(sizeof(T) == dataType.memSize);
    }

    template <typename T>
    void checkReturnType() const {
        if (!detail::isValidTypeCombination<T>(getDataType())) {
            throw BadVariantAccess("Variant does not contain a value convertible to template type");
        }
    }

    void setScalarImpl(void* value, const UA_DataType* type, bool own = false) noexcept;
    void setScalarCopyImpl(const void* value, const UA_DataType* type);
    void setArrayImpl(void* array, size_t size, const UA_DataType* type, bool own = false) noexcept;
    void setArrayCopyImpl(const void* array, size_t size, const UA_DataType* type);
};

/* --------------------------------------- Implementation --------------------------------------- */

template <typename T, Type type>
Variant Variant::fromScalar(T& value) {
    Variant variant;
    if constexpr (isConvertibleToNative<T>()) {
        variant.setScalar<T, type>(value);
    } else {
        variant.setScalarCopy<T, type>(value);
    }
    return variant;
}

template <typename T, Type type>
Variant Variant::fromScalar(const T& value) {
    Variant variant;
    variant.setScalarCopy<T, type>(value);
    return variant;
}

template <typename T, Type type>
Variant Variant::fromArray(T* array, size_t size) {
    Variant variant;
    if constexpr (isConvertibleToNative<T>()) {
        variant.setArray<T, type>(array, size);  // NOLINT, variant isn't modified
    } else {
        variant.setArrayCopy<T, type>(array, size);
    }
    return variant;
}

template <typename T, Type type>
Variant Variant::fromArray(const T* array, size_t size) {
    Variant variant;
    variant.setArrayCopy<T, type>(array, size);
    return variant;
}

template <typename InputIt, Type type>
Variant Variant::fromArray(InputIt first, InputIt last) {
    Variant variant;
    variant.setArrayCopy<InputIt, type>(first, last);
    return variant;
}

template <typename T>
T& Variant::getScalar() {
    return const_cast<T&>(std::as_const(*this).getScalar<T>());  // NOLINT, avoid code duplication
}

template <typename T>
const T& Variant::getScalar() const {
    assertGetNoCopy<T>();
    checkIsScalar();
    checkReturnType<T>();
    checkDataType<T>(*getDataType());
    return *static_cast<const T*>(handle()->data);
}

template <typename T>
T Variant::getScalarCopy() const {
    checkIsScalar();
    checkReturnType<T>();
    return detail::fromNative<T>(handle()->data, getDataType());
}

template <typename T>
T* Variant::getArray() {
    return const_cast<T*>(std::as_const(*this).getArray<T>());  // NOLINT, avoid code duplication
}

template <typename T>
const T* Variant::getArray() const {
    assertGetNoCopy<T>();
    checkIsArray();
    checkReturnType<T>();
    checkDataType<T>(*getDataType());
    return static_cast<const T*>(handle()->data);
}

template <typename T>
std::vector<T> Variant::getArrayCopy() const {
    checkIsArray();
    checkReturnType<T>();
    return detail::fromNativeArray<T>(handle()->data, handle()->arrayLength, getDataType());
}

template <typename T, Type type>
void Variant::setScalar(T& value) noexcept {
    assertSetNoCopy<T>();
    assertNoVariant<type>();
    detail::assertTypeCombination<T, type>();
    if constexpr (detail::IsTypeWrapper<T>::value) {
        setScalarImpl(value.handle(), detail::getUaDataType<type>());
    } else {
        setScalarImpl(&value, detail::getUaDataType<type>());
    }
}

template <typename T>
void Variant::setScalar(T& value, const UA_DataType& dataType) noexcept {
    checkDataType<T>(dataType);
    setScalarImpl(&value, &dataType);
}

template <typename T, Type type>
void Variant::setScalarCopy(const T& value) {
    assertNoVariant<type>();
    detail::assertTypeCombination<T, type>();
    setScalarImpl(
        detail::toNativeAlloc<T, static_cast<TypeIndex>(type)>(value),
        detail::getUaDataType<type>(),
        true  // move ownership
    );
}

template <typename T>
void Variant::setScalarCopy(const T& value, const UA_DataType& dataType) {
    checkDataType<T>(dataType);
    setScalarCopyImpl(&value, &dataType);
}

template <typename T, Type type>
void Variant::setArray(T* array, size_t size) noexcept {
    assertSetNoCopy<T>();
    detail::assertTypeCombination<T, type>();
    setArrayImpl(array, size, detail::getUaDataType<type>());
}

template <typename InputIt, Type type>
void Variant::setArrayCopy(InputIt first, InputIt last) {
    using ValueType = typename std::iterator_traits<InputIt>::value_type;
    detail::assertTypeCombination<ValueType, type>();
    setArrayImpl(
        detail::toNativeArrayAlloc<InputIt, static_cast<TypeIndex>(type)>(first, last),
        std::distance(first, last),
        detail::getUaDataType<type>(),
        true  // move ownership
    );
}

template <typename T, Type type>
void Variant::setArrayCopy(const T* array, size_t size) {
    detail::assertTypeCombination<T, type>();
    if constexpr (detail::isBuiltinType<T>()) {
        setArrayCopyImpl(array, size, detail::getUaDataType<type>());
    } else {
        setArrayCopy<const T*, type>(array, array + size);
    }
}

}  // namespace opcua
