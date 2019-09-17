#pragma once

#include <optional>
#include <memory>

#include "open62541/types.h"
#include "open62541/types_generated_handling.h"

#include "Types.h"
#include "ErrorHandling.h"

namespace opcua {

class Variant {
public:
    Variant() { UA_Variant_init(&variant_); }

    template <typename T>
    Variant(T value) {
        writeScalar<T>(value);
    }

    template <typename T>
    Variant(const std::vector<T>& vector) {
        writeArray<T>(vector);
    }

    ~Variant() {
        clean();
    }

    Variant(const Variant& other)            { UA_Variant_copy(&other.variant_, &variant_); }
    Variant& operator=(const Variant& other) { UA_Variant_copy(&other.variant_, &variant_); return *this; }

    inline bool isEmpty()  const noexcept { return UA_Variant_isEmpty(&variant_); }
    inline bool isScalar() const noexcept { return UA_Variant_isScalar(&variant_); }
    inline bool isArray()  const noexcept { return (variant_.arrayLength > 0 && variant_.data != UA_EMPTY_ARRAY_SENTINEL); }
    
    template <typename T>
    inline bool isType()          const noexcept { return variant_.type == getUaDataType<T>(); }
    inline bool isType(Type type) const noexcept { return variant_.type == getUaDataType(type); }

    template <typename T>
    std::optional<T> readScalar() const noexcept {
        if (!isScalar() || !isType<T>())
            return {};
        return *static_cast<T*>(variant_.data); // copy on purpose
    }

    template <typename T>
    std::optional<std::vector<T>> readArray() const noexcept {
        if (!isArray() || !isType<T>())
            return {};

        // TODO: check dimensions?
        // size_t arrayDimensionsSize;   /* The number of dimensions */
        // UA_UInt32 *arrayDimensions;   /* The length of each dimension */

        auto dataPointer = static_cast<T*>(variant_.data);
        auto result = std::vector<T>(dataPointer, dataPointer + variant_.arrayLength);
        return result;
    }

    template <typename T>
    void writeScalar(T value) {
        clean();
        auto status = UA_Variant_setScalarCopy(&variant_, &value, getUaDataType<T>());
        checkStatusCodeException(status);
        variant_.storageType = UA_VARIANT_DATA;
    }

    template <typename T>
    void writeArray(const std::vector<T>& vector) {
        clean();
        auto status = UA_Variant_setArrayCopy(&variant_,
            vector.data(),
            vector.size(), 
            getUaDataType<T>());
        checkStatusCodeException(status);
        variant_.storageType = UA_VARIANT_DATA;
    }

    template <typename T>
    void writeArrayNoCopy(const std::vector<T>& vector) noexcept {
        clean();
        // UA_Variant_setArray will borrow the vector data compared to UA_Variant_setArrayCopy
        UA_Variant_setArray(&variant_,
            const_cast<T*>(vector.data()), // convert from const void* to void*
            vector.size(), 
            getUaDataType<T>());
        variant_.storageType = UA_VARIANT_DATA_NODELETE;
    }

    inline       UA_Variant* handle()       { return &variant_; }
    inline const UA_Variant* handle() const { return &variant_; }
private:
    inline void clean() noexcept {
        if (!isEmpty() && (variant_.storageType != UA_VARIANT_DATA_NODELETE))
            UA_Variant_deleteMembers(&variant_);
    }

    UA_Variant variant_ {};
};

} // namespace opcua
