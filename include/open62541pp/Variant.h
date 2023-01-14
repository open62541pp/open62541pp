#pragma once

#include <vector>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/Types.h"
#include "open62541pp/open62541.h"

namespace opcua {

// forward declarations
class NodeId;

/**
 * UA_Variant wrapper class.
 */
class Variant : public TypeWrapper<UA_Variant, Type::Variant> {
public:
    using BaseClass::BaseClass;  // inherit contructors

    /// Check if variant is empty
    bool isEmpty() const noexcept;
    /// Check if variant is a scalar
    bool isScalar() const noexcept;
    /// Check if variant is a array
    bool isArray() const noexcept;

    /// Check if variant type is equal to data type
    bool isType(const UA_DataType* type) const noexcept;
    /// Check if variant type is equal to type enum
    bool isType(Type type) const noexcept;
    /// Check if variant type is equal to data type node id
    bool isType(const NodeId& id) const noexcept;

    /// Check if variant type is equal to template type
    template <typename T>
    bool isType() const noexcept {
        return isType(detail::getUaDataType<T>());
    }

    template <typename T>
    T readScalar(const UA_DataType* type) const {
        checkReadScalar(type);
        checkMemSize<T>(type);
        return *static_cast<T*>(handle()->data);  // copy on purpose
    }

    /// Read scalar value with given template type.
    /// An exception is thrown if the variant is not a scalar or not of the given type.
    /// The template type can also be a wrapper type like opcua::String.
    template <typename T>
    T readScalar() const {
        if constexpr (detail::IsTypeWrapper<T>::value) {
            auto result = readScalarImpl<typename T::UaType>();
            return T(result);
        } else {
            return readScalarImpl<T>();
        }
    }

    /// Read array with given template type and return it as a std::vector.
    /// An exception is thrown if the variant is not an array or not of the given type.
    template <typename T>
    std::vector<T> readArray() const {
        return readArrayImpl<T>();
    }

    /// Write scalar value to variant.
    /// The template type can also be a wrapper type like opcua::String.
    template <typename T>
    void setScalar(T value) {
        if constexpr (detail::IsTypeWrapper<T>::value) {
            setScalarImpl<typename T::UaType>(*value.handle());
        } else {
            setScalarImpl<T>(value);
        }
    }

    /// Write array (raw) to variant.
    template <typename T>
    void setArray(const T* array, size_t size) {
        setArrayCopyImpl(array, size, detail::getUaDataType<T>());
    }

    /// Write array (std::vector) to variant.
    template <typename T>
    void setArray(const std::vector<T>& array) {
        setArray(array.data(), array.size());
    }

    /// Write array (std::vector) to variant by passing it's address to the variant (no copy).
    /// If the array values change, the variant values change as well.
    /// Take care of the vectors lifttime. If it gets deconstructed while the variant is still used,
    /// data get's corrupted.
    template <typename T>
    void setArrayNoCopy(std::vector<T>& array) noexcept {
        setArrayImpl(array.data(), array.size(), detail::getUaDataType<T>());
    }

private:
    void checkReadScalar(const UA_DataType* type) const;
    void checkReadArray(const UA_DataType* type) const;

    template <typename T>
    void checkMemSize(const UA_DataType* type) const {
        if (sizeof(T) != type->memSize) {
            throw Exception("Variant contains data of different memory size");
        }
    }

    void setScalarImpl(void* value, const UA_DataType* type);
    void setScalarCopyImpl(const void* value, const UA_DataType* type);
    void setArrayImpl(void* array, size_t size, const UA_DataType* type);
    void setArrayCopyImpl(const void* array, size_t size, const UA_DataType* type);

    template <typename T>
    T readScalarImpl() const {
        const auto* type = detail::getUaDataType<T>();
        checkReadScalar(type);
        checkMemSize<T>(type);
        return *static_cast<T*>(handle()->data);
    }

    template <typename T>
    std::vector<T> readArrayImpl() const {
        const auto* type = detail::getUaDataType<T>();
        checkReadArray(type);
        checkMemSize<T>(type);
        auto* dataPointer = static_cast<T*>(handle()->data);
        return std::vector<T>(dataPointer, dataPointer + handle()->arrayLength);  // NOLINT
    }

    template <typename T>
    void setScalarImpl(T value) {
        setScalarCopyImpl(&value, detail::getUaDataType<T>());
    }
};

}  // namespace opcua
