#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>  // distance
#include <optional>
#include <type_traits>  // enable_if
#include <utility>  // as_const
#include <vector>

#include "open62541pp/Common.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Span.h"
#include "open62541pp/TypeConverter.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/open62541.h"

namespace opcua {

// forward declarations
class NodeId;

/**
 * UA_Variant wrapper class.
 * @ingroup TypeWrapper
 */
class Variant : public TypeWrapper<UA_Variant, UA_TYPES_VARIANT> {
private:
    template <typename T>
    using EnableIfNoSpan = typename std::enable_if_t<!detail::IsSpan<T>::value>;

public:
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit constructors

    /// Create Variant from scalar value (no copy if assignable without conversion).
    template <typename T>
    [[nodiscard]] static Variant fromScalar(T& value);

    /// Create Variant from scalar value with custom data type.
    template <typename T>
    [[nodiscard]] static Variant fromScalar(T& value, const UA_DataType& dataType);

    /// Create Variant from scalar value (copy).
    template <typename T>
    [[nodiscard]] static Variant fromScalar(const T& value);

    /// Create Variant from scalar value with custom data type (copy).
    template <typename T>
    [[nodiscard]] static Variant fromScalar(const T& value, const UA_DataType& dataType);

    /// Create Variant from array (no copy if assignable without conversion).
    template <typename T>
    [[nodiscard]] static Variant fromArray(Span<T> array);

    /// Create Variant from array with custom data type.
    template <typename T>
    [[nodiscard]] static Variant fromArray(Span<T> array, const UA_DataType& dataType);

    /// Create Variant from array (copy).
    template <typename T>
    [[nodiscard]] static Variant fromArray(Span<const T> array);

    /// Create Variant from array with custom data type (copy).
    template <typename T>
    [[nodiscard]] static Variant fromArray(Span<const T> array, const UA_DataType& dataType);

    /// @overload
    template <typename ArrayLike, typename = EnableIfNoSpan<ArrayLike>>
    [[nodiscard]] static Variant fromArray(ArrayLike&& array) {
        return Variant::fromArray(Span{std::forward<ArrayLike>(array)});
    }

    /// @overload
    template <typename ArrayLike, typename = EnableIfNoSpan<ArrayLike>>
    [[nodiscard]] static Variant fromArray(ArrayLike&& array, const UA_DataType& dataType) {
        return Variant::fromArray(Span{std::forward<ArrayLike>(array)}, dataType);
    }

    /// Create Variant from range of elements (copy).
    template <typename InputIt>
    [[nodiscard]] static Variant fromArray(InputIt first, InputIt last);

    /// Check if variant is empty.
    bool isEmpty() const noexcept;
    /// Check if variant is a scalar.
    bool isScalar() const noexcept;
    /// Check if variant is an array.
    bool isArray() const noexcept;

    /// Check if variant type is equal to data type.
    bool isType(const UA_DataType* dataType) const noexcept;
    /// Check if variant type is equal to data type.
    bool isType(const UA_DataType& dataType) const noexcept;
    /// Check if variant type is equal to type enum.
    bool isType(Type type) const noexcept;
    /// Check if variant type is equal to data type node id.
    bool isType(const NodeId& id) const noexcept;

    /// Get data type.
    const UA_DataType* getDataType() const noexcept;

    /// Get variant type.
    std::optional<Type> getVariantType() const noexcept;

    /// Get pointer to the underlying data.
    /// Check the properties and data type before casting it to the actual type.
    /// Use the methods @ref isScalar, @ref isArray, @ref isType / @ref getDataType.
    void* data() noexcept;

    /// @copydoc data
    const void* data() const noexcept;

    [[deprecated("Use the methods isScalar() and data() instead")]] void* getScalar();

    [[deprecated("Use the methods isScalar() and data() instead")]] const void* getScalar() const;

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
    Span<const uint32_t> getArrayDimensions() const noexcept;

    [[deprecated("Use the methods isArray() and data() instead")]] void* getArray();

    [[deprecated("Use the methods isArray() and data() instead")]] const void* getArray() const;

    /// Get array with given template type (only native or wrapper types).
    /// @exception BadVariantAccess If the variant is not an array or not of type `T`.
    template <typename T>
    Span<T> getArray();

    /// Get array with given template type (only native or wrapper types).
    /// @exception BadVariantAccess If the variant is not an array or not of type `T`.
    template <typename T>
    Span<const T> getArray() const;

    /// Get copy of array with given template type and return it as a std::vector.
    /// @exception BadVariantAccess If the variant is not an array or not convertible to `T`.
    template <typename T>
    std::vector<T> getArrayCopy() const;

    /// Assign scalar value to variant.
    template <typename T>
    void setScalar(T& value) noexcept;

    /// Assign scalar value to variant with custom data type.
    template <typename T>
    void setScalar(T& value, const UA_DataType& dataType) noexcept;

    /// Copy scalar value to variant.
    template <typename T>
    void setScalarCopy(const T& value);

    /// Copy scalar value to variant with custom data type.
    template <typename T>
    void setScalarCopy(const T& value, const UA_DataType& dataType);

    /// Assign array to variant.
    template <typename T>
    void setArray(Span<T> array) noexcept;

    /// @overload
    template <typename ArrayLike, typename = EnableIfNoSpan<ArrayLike>>
    void setArray(ArrayLike&& array) noexcept {
        setArray(Span{std::forward<ArrayLike>(array)});
    }

    /// Assign array to variant with custom data type.
    template <typename T>
    void setArray(Span<T> array, const UA_DataType& dataType) noexcept;

    /// @overload
    template <typename ArrayLike, typename = EnableIfNoSpan<ArrayLike>>
    void setArray(ArrayLike&& array, const UA_DataType& dataType) noexcept {
        setArray(Span{std::forward<ArrayLike>(array)}, dataType);
    }

    /// Copy array to variant.
    template <typename T>
    void setArrayCopy(Span<T> array);

    /// @overload
    template <typename ArrayLike, typename = EnableIfNoSpan<ArrayLike>>
    void setArrayCopy(ArrayLike&& array) noexcept {
        setArrayCopy(Span{std::forward<ArrayLike>(array)});
    }

    /// Copy array to variant with custom data type.
    template <typename T>
    void setArrayCopy(Span<T> array, const UA_DataType& dataType);

    /// @overload
    template <typename ArrayLike, typename = EnableIfNoSpan<ArrayLike>>
    void setArrayCopy(ArrayLike&& array, const UA_DataType& dataType) noexcept {
        setArrayCopy(Span{std::forward<ArrayLike>(array)}, dataType);
    }

    /// Copy range of elements as array to variant.
    template <typename InputIt>
    void setArrayCopy(InputIt first, InputIt last);

private:
    template <typename T>
    static constexpr bool isConvertibleToNative() {
        // TypeWrapper<T> is pointer-interconvertible with T
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

    template <typename T>
    static constexpr void assertNoVariant() {
        static_assert(
            !std::is_same_v<T, Variant> && !std::is_same_v<T, UA_Variant>,
            "Variants cannot directly contain another variant"
        );
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

    void setScalarImpl(void* value, const UA_DataType& type, bool own = false) noexcept;
    void setScalarCopyImpl(const void* value, const UA_DataType& type);
    void setArrayImpl(void* array, size_t size, const UA_DataType& type, bool own = false) noexcept;
    void setArrayCopyImpl(const void* array, size_t size, const UA_DataType& type);
};

/* --------------------------------------- Implementation --------------------------------------- */

template <typename T>
Variant Variant::fromScalar(T& value) {
    Variant variant;
    if constexpr (isConvertibleToNative<T>()) {
        variant.setScalar(value);
    } else {
        variant.setScalarCopy(value);
    }
    return variant;
}

template <typename T>
Variant Variant::fromScalar(T& value, const UA_DataType& dataType) {
    Variant variant;
    variant.setScalar(value, dataType);
    return variant;
}

template <typename T>
Variant Variant::fromScalar(const T& value) {
    Variant variant;
    variant.setScalarCopy(value);
    return variant;
}

template <typename T>
Variant Variant::fromScalar(const T& value, const UA_DataType& dataType) {
    Variant variant;
    variant.setScalarCopy(value, dataType);
    return variant;
}

template <typename T>
Variant Variant::fromArray(Span<T> array) {
    Variant variant;
    if constexpr (isConvertibleToNative<T>()) {
        variant.setArray(array);  // NOLINT, variant isn't modified
    } else {
        variant.setArrayCopy(array);
    }
    return variant;
}

template <typename T>
Variant Variant::fromArray(Span<T> array, const UA_DataType& dataType) {
    Variant variant;
    variant.setArray(array, dataType);
    return variant;
}

template <typename T>
Variant Variant::fromArray(Span<const T> array) {
    Variant variant;
    variant.setArrayCopy(array);
    return variant;
}

template <typename T>
Variant Variant::fromArray(Span<const T> array, const UA_DataType& dataType) {
    Variant variant;
    variant.setArrayCopy(array, dataType);
    return variant;
}

template <typename InputIt>
Variant Variant::fromArray(InputIt first, InputIt last) {
    Variant variant;
    variant.setArrayCopy(first, last);
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
    return detail::fromNative<T>(handle()->data, *getDataType());
}

template <typename T>
Span<T> Variant::getArray() {
    assertGetNoCopy<T>();
    checkIsArray();
    checkReturnType<T>();
    checkDataType<T>(*getDataType());
    return {static_cast<T*>(handle()->data), handle()->arrayLength};
}

template <typename T>
Span<const T> Variant::getArray() const {
    assertGetNoCopy<T>();
    checkIsArray();
    checkReturnType<T>();
    checkDataType<T>(*getDataType());
    return {static_cast<const T*>(handle()->data), handle()->arrayLength};
}

template <typename T>
std::vector<T> Variant::getArrayCopy() const {
    checkIsArray();
    checkReturnType<T>();
    return detail::fromNativeArray<T>(handle()->data, handle()->arrayLength, *getDataType());
}

template <typename T>
void Variant::setScalar(T& value) noexcept {
    assertSetNoCopy<T>();
    setScalar(value, detail::guessDataType<T>());
}

template <typename T>
void Variant::setScalar(T& value, const UA_DataType& dataType) noexcept {
    assertNoVariant<T>();
    checkDataType<T>(dataType);
    setScalarImpl(&value, dataType);
}

template <typename T>
void Variant::setScalarCopy(const T& value) {
    assertNoVariant<T>();
    setScalarImpl(
        detail::toNativeAlloc(value),
        detail::guessDataType<T>(),
        true  // move ownership
    );
}

template <typename T>
void Variant::setScalarCopy(const T& value, const UA_DataType& dataType) {
    assertNoVariant<T>();
    checkDataType<T>(dataType);
    setScalarCopyImpl(&value, dataType);
}

template <typename T>
void Variant::setArray(Span<T> array) noexcept {
    assertSetNoCopy<T>();
    setArray(array, detail::guessDataType<T>());
}

template <typename T>
void Variant::setArray(Span<T> array, const UA_DataType& dataType) noexcept {
    assertNoVariant<T>();
    checkDataType<T>(dataType);
    setArrayImpl(array.data(), array.size(), dataType);
}

template <typename T>
void Variant::setArrayCopy(Span<T> array) {
    assertNoVariant<T>();
    if constexpr (detail::isBuiltinType<T>()) {
        setArrayCopyImpl(array.data(), array.size(), detail::guessDataType<T>());
    } else {
        setArrayCopy(array.begin(), array.end());
    }
}

template <typename T>
void Variant::setArrayCopy(Span<T> array, const UA_DataType& dataType) {
    assertNoVariant<T>();
    checkDataType<T>(dataType);
    setArrayCopyImpl(array.data(), array.size(), dataType);
}

template <typename InputIt>
void Variant::setArrayCopy(InputIt first, InputIt last) {
    using ValueType = typename std::iterator_traits<InputIt>::value_type;
    assertNoVariant<ValueType>();
    setArrayImpl(
        detail::toNativeArrayAlloc(first, last),
        std::distance(first, last),
        detail::guessDataTypeFromIterator<InputIt>(),
        true  // move ownership
    );
}

}  // namespace opcua
