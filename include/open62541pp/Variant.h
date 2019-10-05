#pragma once

#include <type_traits>
#include <vector>

#include "open62541/types.h"
#include "open62541/types_generated_handling.h"

#include "Types.h"
#include "TypeWrapper.h"
#include "ErrorHandling.h"

namespace opcua {

class Variant : public TypeWrapper<UA_Variant> {
public:
    using BaseClass::BaseClass; // inherit contructors

    /// Constructor for scalars
    template <typename T>
    Variant(const T& value) { setScalar<T>(value); }

    /// Constructor for arrays
    template <typename T>
    Variant(const std::vector<T>& vector) { setArray<T>(vector); }

    /// Check if variant is empty
    inline bool isEmpty() const noexcept { return UA_Variant_isEmpty(&data_); }
    /// Check if variant is a scalar
    inline bool isScalar() const noexcept { return UA_Variant_isScalar(&data_); }
    /// Check if variant is a array
    inline bool isArray() const noexcept { return (data_.arrayLength > 0 && data_.data != UA_EMPTY_ARRAY_SENTINEL); }

    /// Check if variant type is equal to template type
    template <typename T>
    inline bool isType() const noexcept { return data_.type == getUaDataType<T>(); }
    /// Check if variant type is equal to type argument (enum)
    inline bool isType(Type type) const noexcept { return data_.type == getUaDataType(type); }

    /// Read scalar value with given template type.
    /// An exception is thrown if the variant is not a scalar or not of the given type.
    template <typename T>
    std::enable_if_t<!std::is_convertible_v<T*, TypeWrapperBase*>, T>
    readScalar() const {
        if (!isScalar())
            throw Exception("Variant is not a scalar");
        if (!isType<T>())
            throw Exception("Variant does not contain a scalar of specified return type");
        return *static_cast<T*>(data_.data); // copy on purpose
    }

    /// Overload to read scalar values into wrapper objects.
    template <typename T>
    inline std::enable_if_t<std::is_convertible_v<T*, TypeWrapperBase*>, T>
    readScalar() const {
        return T(readScalar<typename T::UaType>()); // construct type wrapper with result
    }

    /// Read array with given template type and return is as a std::vector-
    /// An exception is thrown if the variant is not an array or not of the given type.
    template <typename T>
    std::vector<T> readArray() const {
        if (!isArray())
            throw Exception("Variant is not an array");
        if (!isType<T>())
            throw Exception("Variant does not contain an array of specified return type");

        // TODO: check dimensions?
        // size_t arrayDimensionsSize;   /* The number of dimensions */
        // UA_UInt32 *arrayDimensions;   /* The length of each dimension */

        auto dataPointer = static_cast<T*>(data_.data);
        auto result      = std::vector<T>(dataPointer, dataPointer + data_.arrayLength);
        return result;
    }

    /// Write scalar value to variant.
    template <typename T,
              typename = std::enable_if_t<!std::is_convertible_v<T*, TypeWrapperBase*>>>
    void setScalar(T value) {
        clear();
        auto status = UA_Variant_setScalarCopy(&data_, &value, getUaDataType<T>());
        checkStatusCodeException(status);
        data_.storageType = UA_VARIANT_DATA;
    }

    /// Overload to write scalar values from wrapper objects to variant.
    template <typename T, Type type>
    inline void setScalar(const TypeWrapper<T, type>& value) {
        setScalar<T>(*value.handle());
    }

    /// Write array (std::vector) to variant.
    template <typename T>
    void setArray(const std::vector<T>& array) {
        clear();
        auto status = UA_Variant_setArrayCopy(&data_,
                                              array.data(),
                                              array.size(),
                                              getUaDataType<T>());
        checkStatusCodeException(status);
        data_.storageType = UA_VARIANT_DATA;
    }

    /// Write array (std::vector) to variant by passing it's address to the variant (no copy).
    /// If the array values change, the variant values change as well.
    /// Take care of the vectors lifttime. If it gets deconstructed while the variant is still used, data get's corrupted.
    template <typename T>
    void setArrayNoCopy(std::vector<T>& array) noexcept {
        clear();
        // UA_Variant_setArray will borrow the vector data compared to UA_Variant_setArrayCopy
        UA_Variant_setArray(&data_,
                            array.data(),
                            array.size(),
                            getUaDataType<T>());
        data_.storageType = UA_VARIANT_DATA_NODELETE;
    }
};

} // namespace opcua
