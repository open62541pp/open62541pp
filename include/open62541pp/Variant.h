#pragma once

#include <optional>
#include <memory>

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
    std::optional<T> readScalar() const noexcept {
        if (!isScalar() || !isType<T>())
            return {};
        return *static_cast<T*>(data_.data); // copy on purpose
    }

    template <typename T>
    std::optional<std::vector<T>> readArray() const noexcept {
        if (!isArray() || !isType<T>())
            return {};

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
    void writeArray(const std::vector<T>& vector) {
        clear();
        auto status = UA_Variant_setArrayCopy(&data_,
            vector.data(),
            vector.size(), 
            getUaDataType<T>());
        checkStatusCodeException(status);
        data_.storageType = UA_VARIANT_DATA;
    }

    template <typename T>
    void writeArrayNoCopy(const std::vector<T>& vector) noexcept {
        clear();
        // UA_Variant_setArray will borrow the vector data compared to UA_Variant_setArrayCopy
        UA_Variant_setArray(&data_,
            const_cast<T*>(vector.data()), // convert from const void* to void*
            vector.size(), 
            getUaDataType<T>());
        data_.storageType = UA_VARIANT_DATA_NODELETE;
    }
};

} // namespace opcua
