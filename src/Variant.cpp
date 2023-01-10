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

bool Variant::isType(Type type) const noexcept {
    return handle()->type == detail::getUaDataType(type);
}

bool Variant::isType(const NodeId& id) const noexcept {
    return handle()->type == detail::getUaDataType(id.handle());
}

}  // namespace opcua
