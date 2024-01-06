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
inline static UA_StatusCode getServiceResult(const Response& response) noexcept {
    return response.responseHeader.serviceResult;
}

/// Completion handler flag for sync client operations.
struct UseSync {};

/// Completion handler flag for async client operations returning `std::future` objects.
struct UseFuture {};

/// Overload for async client requests using callback completion handler.
template <typename Request, typename Response, typename F, typename CompletionHandler>
static auto sendRequest(
    Client& client,
    const Request& request,
    F&& processResponse,
    CompletionHandler&& completionHandler
) {
    static_assert(detail::isNativeType<Request>);
    static_assert(detail::isNativeType<Response>);
    static_assert(std::is_invocable_v<F, Response&>);

    using Result = std::invoke_result_t<F, Response&>;
    if constexpr (std::is_void_v<Result>) {
        static_assert(std::is_invocable_v<CompletionHandler, UA_StatusCode>);
    } else {
        static_assert(std::is_invocable_v<CompletionHandler, UA_StatusCode, Result>);
    }

    using Context = std::tuple<F, CompletionHandler>;

    auto callback = [](UA_Client*, void* userdata, uint32_t /* reqId */, void* responsePtr) {
        assert(userdata != nullptr);
        std::unique_ptr<Context> context{static_cast<Context*>(userdata)};
        auto& handler = std::get<CompletionHandler>(*context);

        try {
            if (responsePtr == nullptr) {
                if constexpr (std::is_void_v<Result>) {
                    std::invoke(handler, UA_STATUSCODE_BADUNEXPECTEDERROR);
                } else {
                    std::invoke(handler, UA_STATUSCODE_BADUNEXPECTEDERROR, Result{});
                }
            }

            Response& response = *static_cast<Response*>(responsePtr);
            UA_StatusCode code = getServiceResult(response);
            auto result = detail::invokeCapture(std::get<F>(*context), response);
            code |= result.code();
            if constexpr (std::is_void_v<Result>) {
                std::invoke(handler, code);
            } else {
                std::invoke(handler, code, std::move(result.get()));
            }
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

/// Overload for async client requests returning `std::future` objects (`UseFuture`flag).
template <typename Request, typename Response, typename F>
static auto sendRequest(Client& client, const Request& request, F&& processResponse, UseFuture) {
    static_assert(detail::isNativeType<Request>);
    static_assert(detail::isNativeType<Response>);
    static_assert(std::is_invocable_v<F, Response&>);

    using Result = std::invoke_result_t<F, Response&>;
    std::promise<Result> promise;
    auto future = promise.get_future();

    sendRequest<Request, Response>(
        client,
        request,
        std::forward<F>(processResponse),
        [p = std::move(promise)](UA_StatusCode code, auto&&... result) mutable {
            if (code != UA_STATUSCODE_GOOD) {
                p.set_exception(std::make_exception_ptr(BadStatus(code)));
            } else {
                p.set_value(std::forward<decltype(result)>(result)...);
            }
        }
    );
    return future;
}

/// Overload for sync client requests (`UseSync` flag).
template <typename Request, typename Response, typename F>
static auto sendRequest(Client& client, const Request& request, F&& processResponse, UseSync) {
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

    detail::throwOnBadStatus(getServiceResult(response));
    return std::invoke(processResponse, response);
}

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
