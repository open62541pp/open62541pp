#pragma once

#include <cassert>
#include <future>
#include <type_traits>

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/TypeConverter.h"  // guessDataType

#include "../open62541_impl.h"

namespace opcua::services {

template <typename Request, typename Response, typename F>
auto sendAsyncRequest(
    Client& client,
    const Request& request,
    F&& transformResponse = [](Response&& response) { return response; }
) {
    static_assert(detail::isNativeType<Request>());
    static_assert(detail::isNativeType<Response>());

    using Result = std::invoke_result_t<F, Response&>;

    struct CallbackContext {
        std::promise<Result> promise;
        F* transform{};
    };

    auto callbackContext = std::make_unique<CallbackContext>();
    callbackContext->transform = &transformResponse;

    auto callback = [](UA_Client*, void* userdata, uint32_t /* requestId */, void* responsePtr) {
        assert(userdata != nullptr);
        auto* context = static_cast<CallbackContext*>(userdata);
        auto& promise = context->promise;
        try {
            if (responsePtr == nullptr) {
                throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
            }
            auto& response = *static_cast<Response*>(responsePtr);
            detail::throwOnBadStatus(response.responseHeader.serviceResult);
            promise.set_value((*context->transform)(response));
        } catch (...) {
            promise.set_exception(std::current_exception());
        }
        delete context;  // NOLINT
    };

    auto future = callbackContext->promise.get_future();
    const auto status = UA_Client_sendAsyncRequest(
        client.handle(),
        &request,
        &detail::guessDataType<Request>(),
        callback,
        &detail::guessDataType<Response>(),
        callbackContext.release(),  // userdata, transfer ownership to callback
        nullptr
    );
    detail::throwOnBadStatus(status);
    return future;
}

}  // namespace opcua::services
