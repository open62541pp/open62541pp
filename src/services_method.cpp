#include "open62541pp/services/method.hpp"

#ifdef UA_ENABLE_METHODCALLS

#include "open62541pp/client.hpp"
#include "open62541pp/server.hpp"
#include "open62541pp/ua/types.hpp"

namespace opcua::services {

CallResponse call(Client& connection, const CallRequest& request) noexcept {
    return UA_Client_Service_call(connection.handle(), request);
}

template <>
CallMethodResult call(
    Server& connection,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
) noexcept {
    const auto item = detail::makeCallMethodRequest(objectId, methodId, inputArguments);
    return UA_Server_call(connection.handle(), &item);
}

template <>
CallMethodResult call(
    Client& connection,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
) noexcept {
    auto item = detail::makeCallMethodRequest(objectId, methodId, inputArguments);
    const auto request = detail::makeCallRequest(item);
    auto response = call(connection, asWrapper<CallRequest>(request));
    return detail::wrapSingleResultWithStatus<CallMethodResult>(response);
}

}  // namespace opcua::services

#endif
