#pragma once

#include <algorithm>  // transform
#include <cassert>
#include <cstdint>
#include <iterator>  // distance
#include <optional>
#include <utility>  // move
#include <vector>

#include "open62541pp/Common.h"  // Type
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Span.h"
#include "open62541pp/TypeConverter.h"
#include "open62541pp/TypeRegistry.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/detail/traits.h"
#include "open62541pp/types/NodeId.h"

namespace opcua {

/**
 * Policies for variant factory methods Variant::fromScalar, Variant::fromArray.
 */
enum class VariantPolicy {
    // clang-format off
    Copy,                 ///< Store copy of scalar/array inside the variant.
    Reference,            ///< Store reference to scalar/array inside the variant.
                          ///< Both scalars and arrays must be mutable native/wrapper types.
                          ///< Arrays must store the elements contiguously in memory.
    ReferenceIfPossible,  ///< Favor referencing but fall back to copying if necessary.
    // clang-format on
};

// forward declarations
namespace detail {
template <VariantPolicy>
struct VariantHandler;
}  // namespace detail

/**
 * UA_Variant wrapper class.
 * @ingroup Wrapper
 */
class Variant : public TypeWrapper<UA_Variant, UA_TYPES_VARIANT> {
public:
    using TypeWrapper::TypeWrapper;  // inherit constructors

    /// Create Variant from scalar value.
    /// @tparam Policy Policy (@ref VariantPolicy) how to store the scalar inside the variant
    template <VariantPolicy Policy = VariantPolicy::Copy, typename T>
    [[nodiscard]] static Variant fromScalar(T&& value) {
        Variant var;
        detail::VariantHandler<Policy>::setScalar(var, std::forward<T>(value));
        return var;
    }

    /// Create Variant from scalar value with custom data type.
    /// @tparam Policy Policy (@ref VariantPolicy) how to store the scalar inside the variant
    template <VariantPolicy Policy = VariantPolicy::Copy, typename T>
    [[nodiscard]] static Variant fromScalar(T&& value, const UA_DataType& dataType) {
        Variant var;
        detail::VariantHandler<Policy>::setScalar(var, std::forward<T>(value), dataType);
        return var;
    }

    /// Create Variant from array.
    /// @tparam Policy Policy (@ref VariantPolicy) how to store the array inside the variant
    template <VariantPolicy Policy = VariantPolicy::Copy, typename ArrayLike>
    [[nodiscard]] static Variant fromArray(ArrayLike&& array) {
        using Handler = detail::VariantHandler<Policy>;
        Variant var;
        if constexpr (detail::IsContiguousContainer<ArrayLike>::value) {
            Handler::setArray(var, Span{std::forward<ArrayLike>(array)});
        } else {
            Handler::setArray(var, array.begin(), array.end());
        }
        return var;
    }

    /// Create Variant from array with custom data type.
    /// @tparam Policy Policy (@ref VariantPolicy) how to store the array inside the variant
    template <VariantPolicy Policy = VariantPolicy::Copy, typename ArrayLike>
    [[nodiscard]] static Variant fromArray(ArrayLike&& array, const UA_DataType& dataType) {
        using Handler = detail::VariantHandler<Policy>;
        Variant var;
        if constexpr (detail::IsContiguousContainer<ArrayLike>::value) {
            Handler::setArray(var, Span{std::forward<ArrayLike>(array)}, dataType);
        } else {
            Handler::setArray(var, array.begin(), array.end(), dataType);
        }
        return var;
    }

    /// Create Variant from range of elements (copy required).
    /// @tparam Policy Policy (@ref VariantPolicy) how to store the array inside the variant
    template <VariantPolicy Policy = VariantPolicy::Copy, typename InputIt>
    [[nodiscard]] static Variant fromArray(InputIt first, InputIt last) {
        Variant var;
        detail::VariantHandler<Policy>::setArray(var, first, last);
        return var;
    }

    /// Create Variant from range of elements with custom data type (copy required).
    /// @tparam Policy Policy (@ref VariantPolicy) how to store the array inside the variant
    template <VariantPolicy Policy = VariantPolicy::Copy, typename InputIt>
    [[nodiscard]] static Variant fromArray(
        InputIt first, InputIt last, const UA_DataType& dataType
    ) {
        Variant var;
        detail::VariantHandler<Policy>::setArray(var, first, last, dataType);
        return var;
    }

    /// Check if the variant is empty.
    bool isEmpty() const noexcept {
        return handle()->type == nullptr;
    }

    /// Check if the variant is a scalar.
    bool isScalar() const noexcept {
        return (handle()->arrayLength == 0) &&
               (handle()->data > UA_EMPTY_ARRAY_SENTINEL);  // NOLINT
    }

    /// Check if the variant is an array.
    bool isArray() const noexcept {
        return (handle()->arrayLength > 0) && (handle()->data > UA_EMPTY_ARRAY_SENTINEL);  // NOLINT
    }

    /// Check if the variant type is equal to the provided data type.
    bool isType(const UA_DataType* dataType) const noexcept {
        return (handle()->type != nullptr && dataType != nullptr) &&
               (handle()->type->typeId == dataType->typeId);
    }

    /// Check if the variant type is equal to the provided data type.
    bool isType(const UA_DataType& dataType) const noexcept {
        return isType(&dataType);
    }

    /// Check if the variant type is equal to the provided type enum.
    /// @deprecated Use isType<T>() instead, the Type enum will be removed
    [[deprecated("Use isType<T>() instead, the Type enum will be removed")]]
    bool isType(Type type) const noexcept {
        return isType(UA_TYPES[static_cast<TypeIndex>(type)]);  // NOLINT
    }

    /// Check if the variant type is equal to the provided data type node id.
    bool isType(const NodeId& id) const noexcept {
        return (handle()->type != nullptr) && (handle()->type->typeId == id);
    }

    /// Check if the variant type is equal to the provided template type.
    template <typename T>
    bool isType() const noexcept {
        return isType(opcua::getDataType<T>());
    }

    /// Get data type.
    const UA_DataType* getDataType() const noexcept {
        return handle()->type;
    }

    /// Get variant type.
    /// @deprecated Use getDataType() or isType<T>() instead, the Type enum will be removed
    [[deprecated("Use getDataType() or isType<T>() instead, the Type enum will be removed")]]
    std::optional<Type> getVariantType() const noexcept {
        if (handle()->type != nullptr) {
            const auto typeIndex = handle()->type->typeKind;
            if (typeIndex <= UA_DATATYPEKIND_DIAGNOSTICINFO) {
                return static_cast<Type>(typeIndex);
            }
        }
        return {};
    }

    /// Get pointer to the underlying data.
    /// Check the properties and data type before casting it to the actual type.
    /// Use the methods @ref isScalar, @ref isArray, @ref isType / @ref getDataType.
    void* data() noexcept {
        return handle()->data;
    }

    /// @copydoc data
    const void* data() const noexcept {
        return handle()->data;
    }

    /// @deprecated Use the methods isScalar() and data() instead
    [[deprecated("Use the methods isScalar() and data() instead")]]
    void* getScalar() {
        checkIsScalar();
        return handle()->data;
    }

    /// @deprecated Use the methods isScalar() and data() instead
    [[deprecated("Use the methods isScalar() and data() instead")]]
    const void* getScalar() const {
        checkIsScalar();
        return handle()->data;
    }

    /// Get reference to scalar value with given template type (only native or wrapper types).
    /// @exception BadVariantAccess If the variant is not a scalar or not of type `T`.
    template <typename T>
    T& getScalar() & {
        assertIsNative<T>();
        checkIsScalar();
        checkIsDataType<T>();
        return *static_cast<T*>(handle()->data);
    }

    /// @copydoc getScalar()&
    template <typename T>
    const T& getScalar() const& {
        assertIsNative<T>();
        checkIsScalar();
        checkIsDataType<T>();
        return *static_cast<const T*>(handle()->data);
    }

    /// @copydoc getScalar()&
    template <typename T>
    T&& getScalar() && {
        return std::move(getScalar<T>());
    }

    /// @copydoc getScalar()&
    template <typename T>
    const T&& getScalar() const&& {
        return std::move(getScalar<T>());
    }

    /// Get copy of scalar value with given template type.
    /// @exception BadVariantAccess If the variant is not a scalar or not convertible to `T`.
    template <typename T>
    T getScalarCopy() const {
        assertIsCopyableOrConvertible<T>();
        return getScalarCopyImpl<T>();
    }

    /// Get array length or 0 if variant is not an array.
    size_t getArrayLength() const noexcept {
        return handle()->arrayLength;
    }

    /// Get array dimensions.
    Span<const uint32_t> getArrayDimensions() const noexcept {
        return {handle()->arrayDimensions, handle()->arrayDimensionsSize};
    }

    /// @deprecated Use the methods isArray() and data() instead
    [[deprecated("Use the methods isArray() and data() instead")]]
    void* getArray() {
        checkIsArray();
        return handle()->data;
    }

    /// @deprecated Use the methods isArray() and data() instead
    [[deprecated("Use the methods isArray() and data() instead")]]
    const void* getArray() const {
        checkIsArray();
        return handle()->data;
    }

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

    /// Assign scalar value to variant (no copy).
    template <typename T>
    void setScalar(T& value) noexcept {
        assertIsNative<T>();
        setScalar(value, opcua::getDataType<T>());
    }

    /// Assign scalar value to variant with custom data type (no copy).
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
     * Assign array to variant (no copy).
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
     * Assign array to variant with custom data type (no copy).
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

    void checkIsScalar() const {
        if (!isScalar()) {
            throw BadVariantAccess("Variant is not a scalar");
        }
    }

    void checkIsArray() const {
        if (!isArray()) {
            throw BadVariantAccess("Variant is not an array");
        }
    }

    template <typename T>
    void checkIsDataType() const {
        const auto* dt = getDataType();
        if (dt == nullptr || dt->typeId != opcua::getDataType<T>().typeId) {
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

/* --------------------------------------- Variant handler -------------------------------------- */

namespace detail {

template <>
struct VariantHandler<VariantPolicy::Copy> {
    template <typename T>
    static void setScalar(Variant& var, const T& value) {
        var.setScalarCopy(value);
    }

    template <typename T>
    static void setScalar(Variant& var, const T& value, const UA_DataType& dtype) {
        var.setScalarCopy(value, dtype);
    }

    template <typename T>
    static void setArray(Variant& var, Span<T> array) {
        var.setArrayCopy(array.begin(), array.end());
    }

    template <typename T>
    static void setArray(Variant& var, Span<T> array, const UA_DataType& dtype) {
        var.setArrayCopy(array.begin(), array.end(), dtype);
    }

    template <typename InputIt>
    static void setArray(Variant& var, InputIt first, InputIt last) {
        var.setArrayCopy(first, last);
    }

    template <typename InputIt>
    static void setArray(Variant& var, InputIt first, InputIt last, const UA_DataType& dtype) {
        var.setArrayCopy(first, last, dtype);
    }
};

template <>
struct VariantHandler<VariantPolicy::Reference> {
    template <typename T>
    static void setScalar(Variant& var, T& value) noexcept {
        var.setScalar(value);
    }

    template <typename T>
    static void setScalar(Variant& var, T& value, const UA_DataType& dtype) noexcept {
        var.setScalar(value, dtype);
    }

    template <typename T>
    static void setArray(Variant& var, Span<T> array) noexcept {
        var.setArray(array);
    }

    template <typename T>
    static void setArray(Variant& var, Span<T> array, const UA_DataType& dtype) noexcept {
        var.setArray(array, dtype);
    }
};

template <>
struct VariantHandler<VariantPolicy::ReferenceIfPossible> : VariantHandler<VariantPolicy::Copy> {
    using VariantHandler<VariantPolicy::Copy>::setScalar;
    using VariantHandler<VariantPolicy::Copy>::setArray;

    template <typename T>
    static void setScalar(Variant& var, T& value) noexcept(detail::isRegisteredType<T>) {
        if constexpr (detail::isRegisteredType<T>) {
            var.setScalar(value);
        } else {
            var.setScalarCopy(value);
        }
    }

    template <typename T>
    static void setScalar(Variant& var, T& value, const UA_DataType& dtype) noexcept {
        var.setScalar(value, dtype);
    }

    template <typename T>
    static void setArray(Variant& var, Span<T> array) noexcept(detail::isRegisteredType<T>) {
        if constexpr (detail::isRegisteredType<T>) {
            var.setArray(array);
        } else {
            var.setArrayCopy(array);
        }
    }

    template <typename T>
    static void setArray(Variant& var, Span<T> array, const UA_DataType& dtype) noexcept {
        var.setArray(array, dtype);
    }

    template <typename T>
    static void setArray(Variant& var, Span<const T> array) {
        var.setArrayCopy(array.begin(), array.end());
    }

    template <typename T>
    static void setArray(Variant& var, Span<const T> array, const UA_DataType& dtype) {
        var.setArrayCopy(array.begin(), array.end(), dtype);
    }
};

}  // namespace detail

}  // namespace opcua
