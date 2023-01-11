#include "open62541pp/Variant.h"

#include "open62541pp/NodeId.h"

namespace opcua {

bool Variant::isEmpty() const noexcept {
    return UA_Variant_isEmpty(handle());
}

bool Variant::isScalar() const noexcept {
    return UA_Variant_isScalar(handle());
}

bool Variant::isArray() const noexcept {
    return (handle()->arrayLength > 0) && (handle()->data != UA_EMPTY_ARRAY_SENTINEL);  // NOLINT
}

bool Variant::isType(const UA_DataType* type) const noexcept {
    return handle()->type == type;
}

bool Variant::isType(Type type) const noexcept {
    return isType(detail::getUaDataType(type));
}

bool Variant::isType(const NodeId& id) const noexcept {
    return isType(detail::getUaDataType(id.handle()));
}

void Variant::checkReadScalar(const UA_DataType* type) const {
    if (!isScalar()) {
        throw Exception("Variant is not a scalar");
    }
    if (!isType(type)) {
        throw Exception("Variant does not contain a scalar of specified return type");
    }
}

void Variant::checkReadArray(const UA_DataType* type) const {
    if (!isArray()) {
        throw Exception("Variant is not an array");
    }
    if (!isType(type)) {
        throw Exception("Variant does not contain an array of specified return type");
    }
}

void Variant::setScalarImpl(void* value, const UA_DataType* type) {
    clear();
    // UA_Variant_setScalar will borrow data compared to UA_Variant_setScalarCopy
    UA_Variant_setScalar(handle(), value, type);
    handle()->storageType = UA_VARIANT_DATA_NODELETE;
}

void Variant::setScalarCopyImpl(const void* value, const UA_DataType* type) {
    clear();
    const auto status = UA_Variant_setScalarCopy(handle(), value, type);
    detail::checkStatusCodeException(status);
    handle()->storageType = UA_VARIANT_DATA;
}

void Variant::setArrayImpl(void* array, size_t size, const UA_DataType* type) {
    clear();
    // UA_Variant_setArray will borrow data compared to UA_Variant_setArrayCopy
    UA_Variant_setArray(handle(), array, size, type);
    handle()->storageType = UA_VARIANT_DATA_NODELETE;
}

void Variant::setArrayCopyImpl(const void* array, size_t size, const UA_DataType* type) {
    clear();
    const auto status = UA_Variant_setArrayCopy(handle(), array, size, type);
    detail::checkStatusCodeException(status);
    handle()->storageType = UA_VARIANT_DATA;
}

}  // namespace opcua
