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
    if (handle()->type == nullptr) {
        return false;
    }
    return handle()->type->typeIndex == static_cast<uint16_t>(type);
}

bool Variant::isType(const NodeId& id) const noexcept {
    return isType(detail::getUaDataType(id.handle()));
}

std::optional<Type> Variant::getVariantType() const noexcept {
    if (handle()->type == nullptr) {
        return {};
    }
    return static_cast<Type>(handle()->type->typeIndex);
}

size_t Variant::getArrayLength() const noexcept {
    return isArray() ? handle()->arrayLength : 0;
}

std::vector<uint32_t> Variant::getArrayDimensions() const {
    if (!isArray()) {
        return {};
    }
    const auto* ptr = handle()->arrayDimensions;
    return {ptr, ptr + handle()->arrayDimensionsSize};  // NOLINT
}

void Variant::checkIsScalar() const {
    if (!isScalar()) {
        throw BadVariantAccess("Variant is not a scalar");
    }
}

void Variant::checkIsArray() const {
    if (!isArray()) {
        throw BadVariantAccess("Variant is not an array");
    }
}

void Variant::setScalarImpl(void* value, const UA_DataType* type, bool own) noexcept {
    clear();
    UA_Variant_setScalar(handle(), value, type);
    handle()->storageType = own ? UA_VARIANT_DATA : UA_VARIANT_DATA_NODELETE;
}

void Variant::setScalarCopyImpl(const void* value, const UA_DataType* type) {
    clear();
    const auto status = UA_Variant_setScalarCopy(handle(), value, type);
    detail::throwOnBadStatus(status);
    handle()->storageType = UA_VARIANT_DATA;
}

void Variant::setArrayImpl(void* array, size_t size, const UA_DataType* type, bool own) noexcept {
    clear();
    UA_Variant_setArray(handle(), array, size, type);
    handle()->storageType = own ? UA_VARIANT_DATA : UA_VARIANT_DATA_NODELETE;
}

void Variant::setArrayCopyImpl(const void* array, size_t size, const UA_DataType* type) {
    clear();
    const auto status = UA_Variant_setArrayCopy(handle(), array, size, type);
    detail::throwOnBadStatus(status);
    handle()->storageType = UA_VARIANT_DATA;
}

}  // namespace opcua
