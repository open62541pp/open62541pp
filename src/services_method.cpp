#include "open62541pp/services/method.hpp"

#ifdef UA_ENABLE_METHODCALLS

#include "open62541pp/client.hpp"
#include "open62541pp/detail/open62541/server.h"
#include "open62541pp/server.hpp"
#include "open62541pp/types_composed.hpp"

namespace opcua::services {

CallResponse call(Client& connection, const CallRequest& request) noexcept {
    return callAsync(connection, request, detail::SyncOperation{});
}

template <>
Result<CallMethodResult> call(
    Server& connection,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
) noexcept {
    const auto item = detail::createCallMethodRequest(objectId, methodId, inputArguments);
    CallMethodResult result = UA_Server_call(connection.handle(), &item);
    return result;
}

template <>
Result<CallMethodResult> call(
    Client& connection,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
) noexcept {
    return callAsync(connection, objectId, methodId, inputArguments, detail::SyncOperation{});
}

}  // namespace opcua::services

#endif
