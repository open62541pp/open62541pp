#pragma once

#include <algorithm>  // transform
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>  // distance, iterator_traits
#include <optional>
#include <type_traits>  // enable_if
#include <utility>  // as_const
#include <vector>

#include "open62541pp/Common.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Span.h"
#include "open62541pp/TypeConverter.h"
#include "open62541pp/TypeRegistry.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/traits.h"
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
    using TypeWrapperBase::TypeWrapperBase;  // inherit constructors

    /// Create Variant from scalar value (no copy if assignable without conversion).
    template <typename T>
    [[nodiscard]] static Variant fromScalar(T& value) {
        Variant variant;
        if constexpr (detail::isRegisteredType<T>) {
            variant.setScalar(value);
        } else {
            variant.setScalarCopy(value);
        }
        return variant;
    }

    /// Create Variant from scalar value with custom data type (no copy).
    template <typename T>
    [[nodiscard]] static Variant fromScalar(T& value, const UA_DataType& dataType) {
        Variant variant;
        variant.setScalar(value, dataType);
        return variant;
    }

    /// Create Variant from scalar value (copy).
    template <typename T>
    [[nodiscard]] static Variant fromScalar(const T& value) {
        Variant variant;
        variant.setScalarCopy(value);
        return variant;
    }

    /// Create Variant from scalar value with custom data type (copy).
    template <typename T>
    [[nodiscard]] static Variant fromScalar(const T& value, const UA_DataType& dataType) {
        Variant variant;
        variant.setScalarCopy(value, dataType);
        return variant;
    }

    /// Create Variant from array (no copy if assignable).
    template <typename ArrayLike>
    [[nodiscard]] static Variant fromArray(ArrayLike&& array) {
        using ValueType = typename std::remove_reference_t<ArrayLike>::value_type;
        Variant variant;
        if constexpr (isAssignableArray<decltype(array)>() && detail::isRegisteredType<ValueType>) {
            variant.setArray(std::forward<ArrayLike>(array));
        } else {
            variant.setArrayCopy(std::forward<ArrayLike>(array));
        }
        return variant;
    }

    /// Create Variant from array with custom data type (no copy if assignable).
    template <typename ArrayLike>
    [[nodiscard]] static Variant fromArray(ArrayLike&& array, const UA_DataType& dataType) {
        Variant variant;
        if constexpr (isAssignableArray<decltype(array)>()) {
            variant.setArray(std::forward<ArrayLike>(array), dataType);
        } else {
            variant.setArrayCopy(std::forward<ArrayLike>(array), dataType);
        }
        return variant;
    }

    /// Create Variant from range of elements (copy).
    template <typename InputIt>
    [[nodiscard]] static Variant fromArray(InputIt first, InputIt last) {
        Variant variant;
        variant.setArrayCopy(first, last);
        return variant;
    }

    /// Create Variant from range of elements with custom data type (copy).
    template <typename InputIt>
    [[nodiscard]] static Variant fromArray(
        InputIt first, InputIt last, const UA_DataType& dataType
    ) {
        Variant variant;
        variant.setArrayCopy(first, last, dataType);
        return variant;
    }

    /// Check if the variant is empty.
    bool isEmpty() const noexcept;
    /// Check if the variant is a scalar.
    bool isScalar() const noexcept;
    /// Check if the variant is an array.
    bool isArray() const noexcept;

    /// Check if the variant type is equal to the provided data type.
    bool isType(const UA_DataType* dataType) const noexcept;
    /// Check if the variant type is equal to the provided data type.
    bool isType(const UA_DataType& dataType) const noexcept;
    /// Check if the variant type is equal to the provided type enum.
    [[deprecated("Use isType<T>() instead, the Type enum will be removed")]]
    bool isType(Type type) const noexcept;
    /// Check if the variant type is equal to the provided data type node id.
    bool isType(const NodeId& id) const noexcept;

    /// Check if the variant type is equal to the provided template type.
    template <typename T>
    bool isType() const noexcept {
        return isType(opcua::getDataType<T>());
    }

    /// Get data type.
    const UA_DataType* getDataType() const noexcept;

    /// Get variant type.
    [[deprecated("Use getDataType() or isType<T>() instead, the Type enum will be removed")]]
    std::optional<Type> getVariantType() const noexcept;

    /// Get pointer to the underlying data.
    /// Check the properties and data type before casting it to the actual type.
    /// Use the methods @ref isScalar, @ref isArray, @ref isType / @ref getDataType.
    void* data() noexcept;

    /// @copydoc data
    const void* data() const noexcept;

    [[deprecated("Use the methods isScalar() and data() instead")]]
    void* getScalar();

    [[deprecated("Use the methods isScalar() and data() instead")]]
    const void* getScalar() const;

    /// Get reference to scalar value with given template type (only native or wrapper types).
    /// @exception BadVariantAccess If the variant is not a scalar or not of type `T`.
    template <typename T>
    T& getScalar() {
        return const_cast<T&>(std::as_const(*this).getScalar<T>());  // NOLINT
    }

    /// Get const reference to scalar value with given template type (only native or wrapper types).
    /// @exception BadVariantAccess If the variant is not a scalar or not of type `T`.
    template <typename T>
    const T& getScalar() const {
        assertIsNative<T>();
        checkIsScalar();
        checkIsDataType<T>();
        return *static_cast<const T*>(handle()->data);
    }

    /// Get copy of scalar value with given template type.
    /// @exception BadVariantAccess If the variant is not a scalar or not convertible to `T`.
    template <typename T>
    T getScalarCopy() const {
        assertIsCopyableOrConvertible<T>();
        return getScalarCopyImpl<T>();
    }

    /// Get array length or 0 if variant is not an array.
    size_t getArrayLength() const noexcept;

    /// Get array dimensions.
    Span<const uint32_t> getArrayDimensions() const noexcept;

    [[deprecated("Use the methods isArray() and data() instead")]]
    void* getArray();

    [[deprecated("Use the methods isArray() and data() instead")]]
    const void* getArray() const;

    /// Get array with given template type (only native or wrapper types).
    /// @exception BadVariantAccess If the variant is not an array or not of type `T`.
    template <typename T>
    Span<T> getArray() {
        assertIsNative<T>();
        checkIsArray();
        checkIsDataType<T>();
        return Span<T>(static_cast<T*>(handle()->data), handle()->arrayLength);
    }

    /// Get array with given template type (only native or wrapper types).
    /// @exception BadVariantAccess If the variant is not an array or not of type `T`.
    template <typename T>
    Span<const T> getArray() const {
        assertIsNative<T>();
        checkIsArray();
        checkIsDataType<T>();
        return Span<const T>(static_cast<const T*>(handle()->data), handle()->arrayLength);
    }

    /// Get copy of array with given template type and return it as a std::vector.
    /// @exception BadVariantAccess If the variant is not an array or not convertible to `T`.
    template <typename T>
    std::vector<T> getArrayCopy() const {
        assertIsCopyableOrConvertible<T>();
        return getArrayCopyImpl<T>();
    }

    /// Assign scalar value to variant.
    template <typename T>
    void setScalar(T& value) noexcept {
        assertIsNative<T>();
        setScalar(value, opcua::getDataType<T>());
    }

    /// Assign scalar value to variant with custom data type.
    template <typename T>
    void setScalar(T& value, const UA_DataType& dataType) noexcept {
        setScalarImpl(&value, dataType, UA_VARIANT_DATA_NODELETE);
    }

    /// Copy scalar value to variant.
    template <typename T>
    void setScalarCopy(const T& value) {
        assertIsCopyableOrConvertible<T>();
        if constexpr (detail::isRegisteredType<T>) {
            setScalarCopyImpl(value, opcua::getDataType<T>());
        } else {
            setScalarCopyConvertImpl(value);
        }
    }

    /// Copy scalar value to variant with custom data type.
    template <typename T>
    void setScalarCopy(const T& value, const UA_DataType& dataType) {
        setScalarCopyImpl(value, dataType);
    }

    /**
     * Assign array to variant.
     * @param array Container with a contiguous sequence of elements.
     *              For example `std::array`, `std::vector` or `Span`.
     *              The underlying array must be accessible with `std::data` and `std::size`.
     */
    template <typename ArrayLike>
    void setArray(ArrayLike&& array) noexcept {
        using ValueType = typename std::remove_reference_t<ArrayLike>::value_type;
        assertIsNative<ValueType>();
        setArray(std::forward<ArrayLike>(array), opcua::getDataType<ValueType>());
    }

    /**
     * Assign array to variant with custom data type.
     * @copydetails setArray
     * @param dataType Custom data type.
     */
    template <typename ArrayLike>
    void setArray(ArrayLike&& array, const UA_DataType& dataType) noexcept {
        static_assert(!isTemporaryArray<decltype(array)>());
        setArrayImpl(std::data(array), std::size(array), dataType, UA_VARIANT_DATA_NODELETE);
    }

    /**
     * Copy array to variant.
     * @param array Iterable container, for example `std::vector`, `std::list` or `Span`.
     *              The container must implement `begin()` and `end()`.
     */
    template <typename ArrayLike>
    void setArrayCopy(const ArrayLike& array) {
        setArrayCopy(array.begin(), array.end());
    }

    /**
     * Copy array to variant with custom data type.
     * @copydetails setArrayCopy
     * @param dataType Custom data type.
     */
    template <typename ArrayLike>
    void setArrayCopy(const ArrayLike& array, const UA_DataType& dataType) {
        setArrayCopy(array.begin(), array.end(), dataType);
    }

    /**
     * Copy range of elements as array to variant.
     */
    template <typename InputIt>
    void setArrayCopy(InputIt first, InputIt last) {
        using ValueType = typename std::iterator_traits<InputIt>::value_type;
        assertIsCopyableOrConvertible<ValueType>();
        if constexpr (detail::isRegisteredType<ValueType>) {
            setArrayCopyImpl(first, last, opcua::getDataType<ValueType>());
        } else {
            setArrayCopyConvertImpl(first, last);
        }
    }

    /**
     * Copy range of elements as array to variant with custom data type.
     */
    template <typename InputIt>
    void setArrayCopy(InputIt first, InputIt last, const UA_DataType& dataType) {
        setArrayCopyImpl(first, last, dataType);
    }

private:
    template <typename ArrayLike>
    static constexpr bool isTemporaryArray() {
        constexpr bool isTemporary = std::is_rvalue_reference_v<ArrayLike>;
        constexpr bool isView = detail::IsSpan<std::remove_reference_t<ArrayLike>>::value;
        return isTemporary && !isView;
    }

    template <typename ArrayLike>
    static constexpr bool isAssignableArray() {
        constexpr bool isTemporary = isTemporaryArray<ArrayLike>();
        constexpr bool isMutable = detail::IsMutableContainer<ArrayLike>::value;
        constexpr bool isContiguous = detail::IsContiguousContainer<ArrayLike>::value;
        return !isTemporary && isMutable && isContiguous;
    }

    template <typename T>
    static constexpr void assertIsNative() {
        static_assert(
            detail::isRegisteredType<T>,
            "Template type must be a native/wrapper type to assign or get scalar/array without copy"
        );
    }

    template <typename T>
    static constexpr void assertIsCopyableOrConvertible() {
        static_assert(
            detail::isRegisteredType<T> || detail::isConvertibleType<T>,
            "Template type must be either a native/wrapper type (copyable) or a convertible type. "
            "If the type is a native type: Provide the data type (UA_DataType) manually "
            "or register the type with a TypeRegistry template specialization. "
            "If the type should be converted: Add a template specialization for TypeConverter."
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
    void checkIsDataType() const {
        if (getDataType() != &opcua::getDataType<T>()) {
            throw BadVariantAccess("Variant does not contain a value convertible to template type");
        }
    }

    template <typename T>
    inline T getScalarCopyImpl() const;
    template <typename T>
    inline std::vector<T> getArrayCopyImpl() const;

    template <typename T>
    inline void setScalarImpl(
        T* data, const UA_DataType& dataType, UA_VariantStorageType storageType
    ) noexcept;
    template <typename T>
    inline void setArrayImpl(
        T* data, size_t arrayLength, const UA_DataType& dataType, UA_VariantStorageType storageType
    ) noexcept;
    template <typename T>
    inline void setScalarCopyImpl(const T& value, const UA_DataType& dataType);
    template <typename T>
    inline void setScalarCopyConvertImpl(const T& value);
    template <typename InputIt>
    inline void setArrayCopyImpl(InputIt first, InputIt last, const UA_DataType& dataType);
    template <typename InputIt>
    inline void setArrayCopyConvertImpl(InputIt first, InputIt last);
};

/* --------------------------------------- Implementation --------------------------------------- */

template <typename T>
T Variant::getScalarCopyImpl() const {
    if constexpr (detail::isRegisteredType<T>) {
        return detail::copy(getScalar<T>(), opcua::getDataType<T>());
    } else {
        using Native = typename TypeConverter<T>::NativeType;
        T result{};
        TypeConverter<T>::fromNative(getScalar<Native>(), result);
        return result;
    }
}

template <typename T>
std::vector<T> Variant::getArrayCopyImpl() const {
    std::vector<T> result(handle()->arrayLength);
    if constexpr (detail::isRegisteredType<T>) {
        auto native = getArray<T>();
        std::transform(native.begin(), native.end(), result.begin(), [](auto&& value) {
            return detail::copy(value, opcua::getDataType<T>());
        });
    } else {
        using Native = typename TypeConverter<T>::NativeType;
        auto native = getArray<Native>();
        for (size_t i = 0; i < native.size(); ++i) {
            TypeConverter<T>::fromNative(native[i], result[i]);
        }
    }
    return result;
}

template <typename T>
void Variant::setScalarImpl(
    T* data, const UA_DataType& dataType, UA_VariantStorageType storageType
) noexcept {
    assertNoVariant<T>();
    assert(sizeof(T) == dataType.memSize);
    clear();
    handle()->type = &dataType;
    handle()->storageType = storageType;
    handle()->data = data;
}

template <typename T>
void Variant::setArrayImpl(
    T* data, size_t arrayLength, const UA_DataType& dataType, UA_VariantStorageType storageType
) noexcept {
    assertNoVariant<T>();
    assert(sizeof(T) == dataType.memSize);
    clear();
    handle()->type = &dataType;
    handle()->storageType = storageType;
    handle()->data = data;
    handle()->arrayLength = arrayLength;
}

template <typename T>
void Variant::setScalarCopyImpl(const T& value, const UA_DataType& dataType) {
    auto native = detail::allocateUniquePtr<T>(dataType);
    *native = detail::copy(value, dataType);
    setScalarImpl(native.release(), dataType, UA_VARIANT_DATA);  // move ownership
}

template <typename T>
void Variant::setScalarCopyConvertImpl(const T& value) {
    using Native = typename TypeConverter<T>::NativeType;
    const auto& dataType = opcua::getDataType<Native>();
    auto native = detail::allocateUniquePtr<Native>(dataType);
    TypeConverter<T>::toNative(value, *native);
    setScalarImpl(native.release(), dataType, UA_VARIANT_DATA);  // move ownership
}

template <typename InputIt>
void Variant::setArrayCopyImpl(InputIt first, InputIt last, const UA_DataType& dataType) {
    using ValueType = typename std::iterator_traits<InputIt>::value_type;
    const size_t size = std::distance(first, last);
    auto native = detail::allocateArrayUniquePtr<ValueType>(size, dataType);
    std::transform(first, last, native.get(), [&](const ValueType& value) {
        return detail::copy(value, dataType);
    });
    setArrayImpl(native.release(), size, dataType, UA_VARIANT_DATA);  // move ownership
}

template <typename InputIt>
void Variant::setArrayCopyConvertImpl(InputIt first, InputIt last) {
    using ValueType = typename std::iterator_traits<InputIt>::value_type;
    using Native = typename TypeConverter<ValueType>::NativeType;
    const auto& dataType = opcua::getDataType<Native>();
    const size_t size = std::distance(first, last);
    auto native = detail::allocateArrayUniquePtr<Native>(size, dataType);
    for (size_t i = 0; i < size; ++i) {
        TypeConverter<ValueType>::toNative(*first++, native.get()[i]);  // NOLINT
    }
    setArrayImpl(native.release(), size, dataType, UA_VARIANT_DATA);  // move ownership
}

}  // namespace opcua
