#include "open62541pp/types/Variant.h"

#include "open62541pp/detail/helper.h"
#include "open62541pp/types/NodeId.h"

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
    return handle()->type == detail::getUaDataType(type);
}

bool Variant::isType(const NodeId& id) const noexcept {
    return isType(detail::getUaDataType(id));
}

std::optional<Type> Variant::getVariantType() const noexcept {
    // UA_DataType typeIndex member was removed in open62541 v1.3
    // https://github.com/open62541/open62541/pull/4477
    // https://github.com/open62541/open62541/issues/4960
    for (size_t typeIndex = 0; typeIndex < detail::builtinTypesCount; ++typeIndex) {
        if (handle()->type == detail::getUaDataType(typeIndex)) {
            return static_cast<Type>(typeIndex);
        }
    }
    return {};
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
