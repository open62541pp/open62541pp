#pragma once

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

    template <typename T>
    Variant(const T& value) { writeScalar<T>(value); }
    template <typename T>
    Variant(const std::vector<T>& vector) { writeArray<T>(vector); }

    inline bool isEmpty()  const noexcept { return UA_Variant_isEmpty(&data_); }
    inline bool isScalar() const noexcept { return UA_Variant_isScalar(&data_); }
    inline bool isArray()  const noexcept { return (data_.arrayLength > 0 && data_.data != UA_EMPTY_ARRAY_SENTINEL); }
    
    template <typename T>
    inline bool isType()          const noexcept { return data_.type == getUaDataType<T>(); }
    inline bool isType(Type type) const noexcept { return data_.type == getUaDataType(type); }

    template <typename T>
    T readScalar() const {
        if (!isScalar())
            throw Exception("Variant is not a scalar");
        if (!isType<T>())
            throw Exception("Variant does not contain a scalar of specified return type");
        return *static_cast<T*>(data_.data); // copy on purpose
    }

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
        auto result = std::vector<T>(dataPointer, dataPointer + data_.arrayLength);
        return result;
    }

    template <typename T>
    void writeScalar(T value) {
        clear();
        auto status = UA_Variant_setScalarCopy(&data_, &value, getUaDataType<T>());
        checkStatusCodeException(status);
        data_.storageType = UA_VARIANT_DATA;
    }

    template <typename T>
    void writeArray(const std::vector<T>& array) {
        clear();
        auto status = UA_Variant_setArrayCopy(&data_,
            array.data(),
            array.size(), 
            getUaDataType<T>());
        checkStatusCodeException(status);
        data_.storageType = UA_VARIANT_DATA;
    }

    template <typename T>
    void writeArrayNoCopy(std::vector<T>& array) noexcept {
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
