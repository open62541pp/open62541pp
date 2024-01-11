#pragma once

#include "open62541pp/Span.h"
#include "open62541pp/TypeWrapper.h"  // asNative
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

namespace opcua::detail {

inline UA_CallMethodRequest createCallMethodRequest(
    const NodeId& objectId, const NodeId& methodId, Span<const Variant> inputArguments
) {
    UA_CallMethodRequest request{};
    request.objectId = objectId;
    request.methodId = methodId;
    request.inputArguments = asNative(const_cast<Variant*>(inputArguments.data()));  // NOLINT
    request.inputArgumentsSize = inputArguments.size();
    return request;
}

}  // namespace opcua::detail
