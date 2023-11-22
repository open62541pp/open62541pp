#pragma once

#include <cassert>
#include <future>
#include <type_traits>
#include <utility>  // exchange, move

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/TypeConverter.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/helper.h"

#include "../open62541_impl.h"

namespace opcua::services {

struct MoveResponse {
    template <typename Response>
    [[nodiscard]] constexpr auto operator()(Response&& value) noexcept {
        return std::exchange(std::forward<Response>(value), {});
    }
};

struct ClientService {
    template <typename Request, typename Response, typename F>
    static auto sendRequest(Client& client, const Request& request, F&& processResponse) {
        static_assert(detail::isNativeType<Request>);
        static_assert(detail::isNativeType<Response>);
        static_assert(std::is_invocable_v<F, Response&>);

        using Result = std::invoke_result_t<F, Response&>;

        Response response{};
        auto clearResponse = [&] { detail::clear(response, detail::guessDataType<Response>()); };

        __UA_Client_Service(
            client.handle(),
            &request,
            &detail::guessDataType<Request>(),
            &response,
            &detail::guessDataType<Response>()
        );

        try {
            detail::throwOnBadStatus(response.responseHeader.serviceResult);
            if constexpr (std::is_void_v<Result>) {
                std::invoke(processResponse, response);
                clearResponse();
                return;
            } else {
                auto result = std::invoke(processResponse, response);
                clearResponse();
                return result;
            }
        } catch (...) {
            clearResponse();
            throw;
        }
    }
};

struct ClientServiceAsync {
    template <typename Request, typename Response, typename F>
    static auto sendRequest(Client& client, const Request& request, F&& processResponse) {
        static_assert(detail::isNativeType<Request>);
        static_assert(detail::isNativeType<Response>);
        static_assert(std::is_invocable_v<F, Response&>);

        using Result = std::invoke_result_t<F, Response&>;

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
                detail::throwOnBadStatus(response.responseHeader.serviceResult);
                if constexpr (std::is_void_v<Result>) {
                    std::invoke(context->process, response);
                    promise.set_value();
                } else {
                    promise.set_value(std::invoke(context->process, response));
                }
            } catch (...) {
                promise.set_exception(std::current_exception());
            }
            delete context;  // NOLINT
        };

        auto future = callbackContext->promise.get_future();
        const auto status = __UA_Client_AsyncService(
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
};

template <typename Response>
auto& getSingleResultFromResponse(Response& response) {
    static_assert(detail::isNativeType<Response> || detail::isTypeWrapper<Response>);
    if constexpr (detail::isNativeType<Response>) {
        if (response.results == nullptr || response.resultsSize != 1) {
            throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
        }
        return *response.results;
    }
    if constexpr (detail::isTypeWrapper<Response>) {
        auto results = response.getResults();
        if (results.data() == nullptr || results.size() != 1) {
            throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
        }
        return results[0];
    }
}

}  // namespace opcua::services
