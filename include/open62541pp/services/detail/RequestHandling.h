#pragma once

#include <type_traits>  // remove_const_t

#include "open62541pp/Common.h"
#include "open62541pp/Span.h"
#include "open62541pp/TypeWrapper.h"  // asNative
#include "open62541pp/open62541.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/ExtensionObject.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

namespace opcua::services::detail {

template <typename T>
inline ExtensionObject wrapNodeAttributes(const T& attributes) {
    // NOLINTNEXTLINE, won't be modified
    return ExtensionObject::fromDecoded(const_cast<T&>(attributes));
}

template <typename T>
inline auto* getNativePointer(Span<T> array) noexcept {
    // NOLINTNEXTLINE, request object won't be modified
    return asNative(const_cast<std::remove_const_t<T>*>(array.data()));
}

inline UA_ReadValueId createReadValueId(const NodeId& id, AttributeId attributeId) noexcept {
    UA_ReadValueId item{};
    item.nodeId = *id.handle();
    item.attributeId = static_cast<uint32_t>(attributeId);
    return item;
}

inline UA_ReadRequest createReadRequest(
    TimestampsToReturn timestamps, UA_ReadValueId& item
) noexcept {
    UA_ReadRequest request{};
    request.timestampsToReturn = static_cast<UA_TimestampsToReturn>(timestamps);
    request.nodesToReadSize = 1;
    request.nodesToRead = &item;
    return request;
}

inline UA_ReadRequest createReadRequest(
    TimestampsToReturn timestamps, Span<const ReadValueId> nodesToRead
) noexcept {
    UA_ReadRequest request{};
    request.timestampsToReturn = static_cast<UA_TimestampsToReturn>(timestamps);
    request.nodesToReadSize = nodesToRead.size();
    request.nodesToRead = getNativePointer(nodesToRead);
    return request;
}

inline UA_WriteValue createWriteValue(
    const NodeId& id, AttributeId attributeId, const DataValue& value
) noexcept {
    UA_WriteValue item{};
    item.nodeId = *id.handle();
    item.attributeId = static_cast<uint32_t>(attributeId);
    item.value = *value.handle();  // shallow copy, avoid copy of value
    item.value.hasValue = true;
    return item;
}

inline UA_WriteRequest createWriteRequest(UA_WriteValue& item) noexcept {
    UA_WriteRequest request{};
    request.nodesToWriteSize = 1;
    request.nodesToWrite = &item;
    return request;
}

inline UA_WriteRequest createWriteRequest(Span<const WriteValue> nodesToWrite) noexcept {
    UA_WriteRequest request{};
    request.nodesToWriteSize = nodesToWrite.size();
    request.nodesToWrite = getNativePointer(nodesToWrite);
    return request;
}

inline UA_CallMethodRequest createCallMethodRequest(
    const NodeId& objectId, const NodeId& methodId, Span<const Variant> inputArguments
) noexcept {
    UA_CallMethodRequest request{};
    request.objectId = objectId;
    request.methodId = methodId;
    request.inputArgumentsSize = inputArguments.size();
    request.inputArguments = getNativePointer(inputArguments);
    return request;
}

}  // namespace opcua::services::detail
