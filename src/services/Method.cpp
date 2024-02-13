#include "open62541pp/services/Method.h"

#ifdef UA_ENABLE_METHODCALLS

#include "open62541pp/Server.h"
#include "open62541pp/types/Composed.h"

#include "../open62541_impl.h"

namespace opcua::services {

template <>
std::vector<Variant> call(
    Server& server,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
) {
    UA_CallMethodRequest item = detail::createCallMethodRequest(objectId, methodId, inputArguments);
    CallMethodResult result = UA_Server_call(server.handle(), &item);
    return detail::getOutputArguments(result);
}

template <>
std::vector<Variant> call(
    Client& client,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
) {
    return callAsync(client, objectId, methodId, inputArguments, detail::SyncOperation{});
}

}  // namespace opcua::services

#endif
