#include "open62541pp/services/Method.h"

#ifdef UA_ENABLE_METHODCALLS

#include <cassert>
#include <cstddef>
#include <memory>

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/helper.h"  // getUaDataType
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

#include "../open62541_impl.h"

namespace opcua::services {

template <>
std::vector<Variant> call(
    Server& server,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
) {
    UA_CallMethodRequest request{};
    request.objectId = objectId;
    request.methodId = methodId;
    request.inputArgumentsSize = inputArguments.size();
    request.inputArguments = const_cast<UA_Variant*>(asNative(inputArguments.data()));  // NOLINT

    using Result = TypeWrapper<UA_CallMethodResult, UA_TYPES_CALLMETHODRESULT>;
    const Result result = UA_Server_call(server.handle(), &request);
    detail::throwOnBadStatus(result->statusCode);
    for (size_t i = 0; i < result->inputArgumentResultsSize; ++i) {
        detail::throwOnBadStatus(result->inputArgumentResults[i]);  // NOLINT
    }

    return {
        result->outputArguments,
        result->outputArguments + result->outputArgumentsSize  // NOLINT
    };
}

template <>
std::vector<Variant> call(
    Client& client,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
) {
    size_t outputSize{};
    UA_Variant* output{};
    const auto status = UA_Client_call(
        client.handle(),
        objectId,
        methodId,
        inputArguments.size(),
        asNative(inputArguments.data()),
        &outputSize,
        &output
    );
    std::vector<Variant> result(output, output + outputSize);  // NOLINT
    UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
    detail::throwOnBadStatus(status);
    return result;
}

std::future<std::vector<Variant>> callAsync(
    Client& client,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
) {
    using Result = std::vector<Variant>;
    auto promise = std::make_unique<std::promise<Result>>();
    auto future = promise->get_future();
    const auto status = UA_Client_call_async(
        client.handle(),
        objectId,
        methodId,
        inputArguments.size(),
        asNative(inputArguments.data()),
        [](UA_Client*, void* userdata, uint32_t, UA_CallResponse* response) {
            assert(userdata != nullptr);
            auto* promisePtr = static_cast<std::promise<Result>*>(userdata);
            try {
                detail::throwOnBadStatus(response->responseHeader.serviceResult);
                if (response->resultsSize != 1 || response->results == nullptr) {
                    throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
                }
                auto& result = *response->results;
                detail::throwOnBadStatus(result.statusCode);
                // TODO: check status codes of inputArgumentResults
                promisePtr->set_value(std::vector<Variant>(
                    result.outputArguments,
                    result.outputArguments + result.outputArgumentsSize  // NOLINT
                ));
            } catch (...) {
                promisePtr->set_exception(std::current_exception());
            }
            delete promisePtr;  // NOLINT
        },
        promise.release(),  // userdata
        nullptr
    );
    detail::throwOnBadStatus(status);
    return future;
}

}  // namespace opcua::services

#endif
