#include "open62541pp/services/Method.h"

#ifdef UA_ENABLE_METHODCALLS

#include <cstddef>
#include <iterator>

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

#include "../open62541_impl.h"
#include "ClientService.h"

namespace opcua::services {

inline static UA_CallMethodRequest createCallMethodRequest(
    const NodeId& objectId, const NodeId& methodId, Span<const Variant> inputArguments
) {
    UA_CallMethodRequest request{};
    request.objectId = objectId;
    request.methodId = methodId;
    request.inputArguments = asNative(const_cast<Variant*>(inputArguments.data()));  // NOLINT
    request.inputArgumentsSize = inputArguments.size();
    return request;
}

inline static auto getOutputArguments(UA_CallMethodResult& result) {
    detail::throwOnBadStatus(result.statusCode);
    detail::throwOnBadStatus(result.inputArgumentResults, result.inputArgumentResultsSize);
    return std::vector<Variant>(
        std::make_move_iterator(result.outputArguments),
        std::make_move_iterator(result.outputArguments + result.outputArgumentsSize)  // NOLINT
    );
}

template <>
std::vector<Variant> call(
    Server& server,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
) {
    UA_CallMethodRequest item = createCallMethodRequest(objectId, methodId, inputArguments);
    CallMethodResult result = UA_Server_call(server.handle(), &item);
    return getOutputArguments(result);
}

template <typename CompletionToken>
static auto callImpl(
    Client& client,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments,
    CompletionToken&& token
) {
    UA_CallMethodRequest item = createCallMethodRequest(objectId, methodId, inputArguments);
    UA_CallRequest request{};
    request.methodsToCall = &item;
    request.methodsToCallSize = 1;
    return sendRequest<UA_CallRequest, UA_CallResponse>(
        client,
        request,
        [](UA_CallResponse& response) {
            return getOutputArguments(getSingleResultFromResponse(response));
        },
        std::forward<CompletionToken>(token)
    );
}

template <>
std::vector<Variant> call(
    Client& client,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
) {
    return callImpl(client, objectId, methodId, inputArguments, SyncOperation{});
}

std::future<std::vector<Variant>> callAsync(
    Client& client,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
) {
    return callImpl(client, objectId, methodId, inputArguments, useFuture);
}

void callAsync(
    Client& client,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments,
    AsyncCallback<std::vector<Variant>> callback
) {
    return callImpl(client, objectId, methodId, inputArguments, std::move(callback));
}

}  // namespace opcua::services

#endif
