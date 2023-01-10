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
class Variant : public TypeWrapper<UA_Variant> {
public:
    using BaseClass::BaseClass;  // inherit contructors

    /// Check if variant is empty
    bool isEmpty() const noexcept;
    /// Check if variant is a scalar
    bool isScalar() const noexcept;
    /// Check if variant is a array
    bool isArray() const noexcept;

    /// Check if variant type is equal to template type
    template <typename T>
    bool isType() const noexcept {
        return handle()->type == detail::getUaDataType<T>();
    }

    /// Check if variant type is equal to type argument (enum)
    bool isType(Type type) const noexcept;
    /// Check if variant type is equal to data type node id
    bool isType(const NodeId& id) const noexcept;

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
        if (!isArray()) {
            throw Exception("Variant is not an array");
        }
        if (!isType<T>()) {
            throw Exception("Variant does not contain an array of specified return type");
        }
        auto* dataPointer = static_cast<T*>(handle()->data);
        return std::vector<T>(dataPointer, dataPointer + handle()->arrayLength);  // NOLINT
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
        clear();
        const auto status = UA_Variant_setArrayCopy(
            handle(), array, size, detail::getUaDataType<T>()
        );
        detail::checkStatusCodeException(status);
        handle()->storageType = UA_VARIANT_DATA;
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
        clear();
        // UA_Variant_setArray will borrow the vector data compared to UA_Variant_setArrayCopy
        UA_Variant_setArray(handle(), array.data(), array.size(), detail::getUaDataType<T>());
        handle()->storageType = UA_VARIANT_DATA_NODELETE;
    }

private:
    template <typename T>
    T readScalarImpl() const {
        if (!isScalar()) {
            throw Exception("Variant is not a scalar");
        }
        if (!isType<T>()) {
            throw Exception("Variant does not contain a scalar of specified return type");
        }
        return *static_cast<T*>(handle()->data);  // copy on purpose
    }

    template <typename T>
    void setScalarImpl(T value) {
        clear();
        const auto status = UA_Variant_setScalarCopy(handle(), &value, detail::getUaDataType<T>());
        detail::checkStatusCodeException(status);
        handle()->storageType = UA_VARIANT_DATA;
    }
};

}  // namespace opcua
