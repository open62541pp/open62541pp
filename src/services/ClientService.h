#pragma once

#include <cassert>
#include <future>
#include <tuple>
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

template <typename Response>
inline static void checkServiceResult(Response& response) {
    detail::throwOnBadStatus(response.responseHeader.serviceResult);
}

struct ClientService {
    template <typename Request, typename Response, typename F>
    static auto sendRequest(Client& client, const Request& request, F&& processResponse) {
        static_assert(detail::isNativeType<Request>);
        static_assert(detail::isNativeType<Response>);
        static_assert(std::is_invocable_v<F, Response&>);

        Response response{};
        const auto responseDeleter = detail::ScopeExit([&] {
            detail::clear(response, detail::guessDataType<Response>());
        });

        __UA_Client_Service(
            client.handle(),
            &request,
            &detail::guessDataType<Request>(),
            &response,
            &detail::guessDataType<Response>()
        );

        checkServiceResult(response);
        return std::invoke(processResponse, response);
    }
};

struct ClientServiceAsync {
    template <typename Request, typename Response, typename F>
    static auto sendRequest(Client& client, const Request& request, F&& processResponse) {
        static_assert(detail::isNativeType<Request>);
        static_assert(detail::isNativeType<Response>);
        static_assert(std::is_invocable_v<F, Response&>);

        using Result = std::invoke_result_t<F, Response&>;
        using Promise = std::promise<Result>;
        using Context = std::tuple<Promise, F>;

        auto callback = [](UA_Client*, void* userdata, uint32_t /* reqId */, void* responsePtr) {
            assert(userdata != nullptr);
            auto* context = static_cast<Context*>(userdata);
            auto& [promise, func] = *context;
            try {
                if (responsePtr == nullptr) {
                    throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
                }
                auto& response = *static_cast<Response*>(responsePtr);
                checkServiceResult(response);
                if constexpr (std::is_void_v<Result>) {
                    std::invoke(func, response);
                    promise.set_value();
                } else {
                    promise.set_value(std::invoke(func, response));
                }
            } catch (...) {
                promise.set_exception(std::current_exception());
            }
            delete context;  // NOLINT
        };

        auto context = std::make_unique<Context>(Promise{}, std::forward<F>(processResponse));
        auto future = std::get<Promise>(*context).get_future();

        const auto status = __UA_Client_AsyncService(
            client.handle(),
            &request,
            &detail::guessDataType<Request>(),
            callback,
            &detail::guessDataType<Response>(),
            context.release(),  // userdata, transfer ownership to callback
            nullptr
        );

        detail::throwOnBadStatus(status);
        return future;
    }

    template <typename Request, typename Response, typename F, typename CompletionHandler>
    static auto sendRequestCallbackOnly(
        Client& client,
        const Request& request,
        F&& processResponse,
        CompletionHandler&& completionHandler
    ) {
        static_assert(detail::isNativeType<Request>);
        static_assert(detail::isNativeType<Response>);
        static_assert(std::is_invocable_v<F, Response&>);

        using Result = std::invoke_result_t<F, Response&>;
        static_assert(std::is_invocable_v<CompletionHandler, UA_StatusCode, Result>);

        using Context = std::tuple<F, CompletionHandler>;

        auto callback = [](UA_Client*, void* userdata, uint32_t /* reqId */, void* responsePtr) {
            assert(userdata != nullptr);
            std::unique_ptr<Context> context{static_cast<Context*>(userdata)};

            UA_StatusCode status = UA_STATUSCODE_GOOD;
            Response response{};
            if (responsePtr == nullptr) {
                status = UA_STATUSCODE_BADUNEXPECTEDERROR;
            } else {
                response = *static_cast<Response*>(responsePtr);
            }

            Result result{};
            try {
                checkServiceResult(response);
                result = std::invoke(std::get<F>(*context), response);
            } catch (const BadStatus& e) {
                status = e.code();
            } catch (const std::exception&) {
                status = UA_STATUSCODE_BADUNEXPECTEDERROR;
                // TODO: log exception message
            }

            try {
                std::invoke(std::get<CompletionHandler>(*context), status, result);
            } catch (const std::exception&) {
                // TODO: log exception message
            }
        };

        auto context = std::make_unique<Context>(
            std::forward<F>(processResponse), std::forward<CompletionHandler>(completionHandler)
        );

        const auto status = __UA_Client_AsyncService(
            client.handle(),
            &request,
            &detail::guessDataType<Request>(),
            callback,
            &detail::guessDataType<Response>(),
            context.release(),  // userdata, transfer ownership to callback
            nullptr
        );

        detail::throwOnBadStatus(status);
    }
};

template <typename Response>
auto& getSingleResultFromResponse(Response& response) {
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
