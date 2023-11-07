#pragma once

#include <cassert>
#include <future>
#include <type_traits>
#include <utility>  // move

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/TypeWrapper.h"

#include "../open62541_impl.h"

namespace opcua::services {

template <typename Response>
struct ForwardResponse {
    constexpr Response operator()(Response& response) noexcept {
        return response;
    }

    constexpr Response operator()(Response&& response) noexcept {
        return response;
    }
};

template <typename Request, typename Response, typename Fn>
auto sendRequest(Client& client, const Request& request, Fn&& transformResponse) {
    static_assert(detail::isTypeWrapper<Request>);
    static_assert(detail::isTypeWrapper<Response>);
    static_assert(std::is_invocable_v<Fn, Response&> || std::is_invocable_v<Fn, Response&&>);

    Response response{};
    __UA_Client_Service(
        client.handle(),
        request.handle(),
        &UA_TYPES[Request::getTypeIndex()],
        response.handle(),
        &UA_TYPES[Response::getTypeIndex()]
    );

    detail::throwOnBadStatus(response->responseHeader.serviceResult);
    if constexpr (std::is_invocable_v<Fn, Response&&>) {
        return transformResponse(std::move(response));
    } else {
        return transformResponse(response);
    }
}

template <typename Request, typename Response, typename Fn>
auto sendAsyncRequest(Client& client, const Request& request, Fn&& transformResponse) {
    static_assert(detail::isTypeWrapper<Request>);
    static_assert(detail::isTypeWrapper<Response>);
    static_assert(std::is_invocable_v<Fn, Response&> || std::is_invocable_v<Fn, Response&&>);

    constexpr bool rvalue = std::is_invocable_v<Fn, Response&&>;
    using Result = std::invoke_result_t<Fn, std::conditional_t<rvalue, Response&&, Response&>>;

    struct CallbackContext {
        std::promise<Result> promise;
        Fn transform;
    };

    auto callbackContext = std::make_unique<CallbackContext>();
    callbackContext->transform = std::move(transformResponse);

    auto callback = [](UA_Client*, void* userdata, uint32_t /* requestId */, void* responsePtr) {
        assert(userdata != nullptr);
        auto* context = static_cast<CallbackContext*>(userdata);
        auto& promise = context->promise;
        try {
            if (responsePtr == nullptr) {
                throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
            }
            auto& response = *static_cast<Response*>(responsePtr);
            detail::throwOnBadStatus(response->responseHeader.serviceResult);
            if constexpr (std::is_invocable_v<Fn, Response&&>) {
                Response responseMove;
                std::swap(response, responseMove);
                promise.set_value((*context->transform)(std::move(responseMove)));
            } else {
                if constexpr (std::is_void_v<Result>) {
                    (*context->transform)(response);
                    promise.set_value();
                } else {
                    promise.set_value((*context->transform)(response));
                }
            }
        } catch (...) {
            promise.set_exception(std::current_exception());
        }
        delete context;  // NOLINT
    };

    auto future = callbackContext->promise.get_future();
    const auto status = __UA_Client_AsyncService(
        client.handle(),
        request.handle(),
        &UA_TYPES[Request::getTypeIndex()],
        callback,
        &UA_TYPES[Response::getTypeIndex()],
        callbackContext.release(),  // userdata, transfer ownership to callback
        nullptr
    );
    detail::throwOnBadStatus(status);
    return future;
}

}  // namespace opcua::services
