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

struct ClientService {
    template <typename Request, typename Response, typename F>
    static auto sendRequest(Client& client, const Request& request, F&& processResponse) {
        static_assert(detail::isTypeWrapper<Request>);
        static_assert(detail::isTypeWrapper<Response>);
        static_assert(std::is_invocable_v<F, Response&> || std::is_invocable_v<F, Response&&>);

        Response response{};
        __UA_Client_Service(
            client.handle(),
            request.handle(),
            &UA_TYPES[Request::getTypeIndex()],
            response.handle(),
            &UA_TYPES[Response::getTypeIndex()]
        );

        detail::throwOnBadStatus(response->responseHeader.serviceResult);
        if constexpr (std::is_invocable_v<F, Response&&>) {
            return processResponse(std::move(response));
        } else {
            return processResponse(response);
        }
    }
};

struct ClientServiceAsync {
    template <typename Request, typename Response, typename F>
    static auto sendRequest(Client& client, const Request& request, F&& processResponse) {
        static_assert(detail::isTypeWrapper<Request>);
        static_assert(detail::isTypeWrapper<Response>);
        static_assert(std::is_invocable_v<F, Response&> || std::is_invocable_v<F, Response&&>);

        constexpr bool rvalue = std::is_invocable_v<F, Response&&>;
        using Result = std::invoke_result_t<F, std::conditional_t<rvalue, Response&&, Response&>>;

        struct CallbackContext {
            CallbackContext(F&& func)
                : process(func) {}

            std::promise<Result> promise;
            F process;
        };

        auto callbackContext = std::make_unique<CallbackContext>(std::forward<F>(processResponse));

        auto callback = [](UA_Client*, void* userdata, uint32_t /* reqId */, void* responsePtr) {
            assert(userdata != nullptr);
            auto* context = static_cast<CallbackContext*>(userdata);
            auto& promise = context->promise;
            try {
                if (responsePtr == nullptr) {
                    throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
                }
                auto& response = *static_cast<Response*>(responsePtr);
                detail::throwOnBadStatus(response->responseHeader.serviceResult);
                if constexpr (std::is_invocable_v<F, Response&&>) {
                    promise.set_value((*context->process)(std::move(response)));
                } else {
                    if constexpr (std::is_void_v<Result>) {
                        (*context->process)(response);
                        promise.set_value();
                    } else {
                        promise.set_value((*context->process)(response));
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
};

template <typename Response>
auto& getSingleResultFromResponse(Response& response) {
    static_assert(detail::isTypeWrapper<Response>);
    auto results = response.getResults();
    if (results.data() == nullptr || results.size() != 1) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
    return results[0];
}

}  // namespace opcua::services
