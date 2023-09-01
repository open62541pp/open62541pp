#include "open62541pp/types/Variant.h"

#include "open62541pp/detail/helper.h"
#include "open62541pp/overloads/comparison.h"
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

bool Variant::isType(const UA_DataType* dataType) const noexcept {
    const auto* dt = getDataType();
    if (dt == nullptr || dataType == nullptr) {
        return false;
    }
    if (dt == dataType) {
        return true;
    }
    return dt->typeId == dataType->typeId;
}

bool Variant::isType(const UA_DataType& dataType) const noexcept {
    return isType(&dataType);
}

bool Variant::isType(Type type) const noexcept {
    return isType(&detail::getUaDataType(type));
}

bool Variant::isType(const NodeId& id) const noexcept {
    const auto* dt = getDataType();
    if (dt == nullptr) {
        return false;
    }
    return dt->typeId == id;
}

const UA_DataType* Variant::getDataType() const noexcept {
    return handle()->type;
}

std::optional<Type> Variant::getVariantType() const noexcept {
    // UA_DataType::typeIndex member was removed in open62541 v1.3
    // use typeKind instead: https://github.com/open62541/open62541/issues/4960
    static_assert(UA_TYPES_BOOLEAN == UA_DATATYPEKIND_BOOLEAN);
    static_assert(UA_TYPES_VARIANT == UA_DATATYPEKIND_VARIANT);
    if (getDataType() != nullptr) {
        const auto typeIndex = getDataType()->typeKind;
        if (typeIndex <= UA_DATATYPEKIND_DIAGNOSTICINFO) {
            return static_cast<Type>(typeIndex);
        }
    }
    return {};
}

void* Variant::data() noexcept {
    return handle()->data;
}

const void* Variant::data() const noexcept {
    return handle()->data;
}

void* Variant::getScalar() {
    checkIsScalar();
    return handle()->data;
}

const void* Variant::getScalar() const {
    checkIsScalar();
    return handle()->data;
}

size_t Variant::getArrayLength() const noexcept {
    return isArray() ? handle()->arrayLength : 0;
}

void* Variant::getArray() {
    checkIsArray();
    return handle()->data;
}

const void* Variant::getArray() const {
    checkIsArray();
    return handle()->data;
}

Span<const uint32_t> Variant::getArrayDimensions() const noexcept {
    if (!isArray()) {
        return {};
    }
    return {handle()->arrayDimensions, handle()->arrayDimensionsSize};
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

void Variant::setScalarImpl(void* value, const UA_DataType& type, bool own) noexcept {
    clear();
    UA_Variant_setScalar(handle(), value, &type);
    handle()->storageType = own ? UA_VARIANT_DATA : UA_VARIANT_DATA_NODELETE;
}

void Variant::setScalarCopyImpl(const void* value, const UA_DataType& type) {
    clear();
    const auto status = UA_Variant_setScalarCopy(handle(), value, &type);
    detail::throwOnBadStatus(status);
    handle()->storageType = UA_VARIANT_DATA;
}

void Variant::setArrayImpl(void* array, size_t size, const UA_DataType& type, bool own) noexcept {
    clear();
    UA_Variant_setArray(handle(), array, size, &type);
    handle()->storageType = own ? UA_VARIANT_DATA : UA_VARIANT_DATA_NODELETE;
}

void Variant::setArrayCopyImpl(const void* array, size_t size, const UA_DataType& type) {
    clear();
    const auto status = UA_Variant_setArrayCopy(handle(), array, size, &type);
    detail::throwOnBadStatus(status);
    handle()->storageType = UA_VARIANT_DATA;
}

}  // namespace opcua
