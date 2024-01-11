#pragma once

#include <cassert>
#include <future>
#include <tuple>
#include <type_traits>
#include <utility>  // exchange, move

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/TypeRegistry.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/async.h"
#include "open62541pp/detail/Result.h"
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

template <typename Request, typename Response, typename TransformResponse, typename CompletionToken>
auto sendRequest(
    Client& client,
    const Request& request,
    TransformResponse&& transformResponse,
    CompletionToken&& token
) {
    static_assert(std::is_invocable_v<TransformResponse, Response&>);
    using Result = std::invoke_result_t<TransformResponse, Response&>;

    auto init = [&](auto completionHandler, auto transform) {
        using CompletionHandler = decltype(completionHandler);
        using Context = std::tuple<TransformResponse, CompletionHandler>;

        auto callback = [](UA_Client*, void* userdata, uint32_t /* reqId */, void* responsePtr) {
            assert(userdata != nullptr);
            std::unique_ptr<Context> context{static_cast<Context*>(userdata)};
            auto& handler = std::get<CompletionHandler>(*context);

            try {
                if (responsePtr == nullptr) {
                    if constexpr (std::is_void_v<Result>) {
                        std::invoke(handler, UA_STATUSCODE_BADUNEXPECTEDERROR);
                    } else {
                        Result emptyResult{};
                        std::invoke(handler, UA_STATUSCODE_BADUNEXPECTEDERROR, emptyResult);
                    }
                }

                Response& response = *static_cast<Response*>(responsePtr);
                UA_StatusCode code = getServiceResult(response);
                auto result = detail::tryInvoke(std::get<TransformResponse>(*context), response);
                code |= result.code();
                if constexpr (std::is_void_v<Result>) {
                    std::invoke(handler, code);
                } else {
                    std::invoke(handler, code, std::move(*result));
                }
            } catch (const std::exception&) {
                // TODO: log exception message
            }
        };

        auto context = std::make_unique<Context>(
            std::move(transform), std::move(completionHandler)
        );

        const auto status = __UA_Client_AsyncService(
            client.handle(),
            &request,
            &detail::getDataType<Request>(),
            callback,
            &detail::getDataType<Response>(),
            context.release(),  // userdata, transfer ownership to callback
            nullptr
        );
        detail::throwOnBadStatus(status);
    };

    return asyncInitiate<Result>(
        init,
        std::forward<CompletionToken>(token),
        std::forward<TransformResponse>(transformResponse)
    );
}

/// Completion token for sync client operations.
struct SyncOperation {};

/// Overload for sync client requests.
template <typename Request, typename Response, typename F>
static auto sendRequest(
    Client& client, const Request& request, F&& processResponse, SyncOperation
) {
    Response response{};
    const auto responseDeleter = detail::ScopeExit([&] {
        detail::clear(response, detail::getDataType<Response>());
    });

    __UA_Client_Service(
        client.handle(),
        &request,
        &detail::getDataType<Request>(),
        &response,
        &detail::getDataType<Response>()
    );

    detail::throwOnBadStatus(getServiceResult(response));
    return std::invoke(processResponse, response);
}

template <typename Response>
auto& getSingleResultFromResponse(Response& response) {
    if constexpr (detail::isTypeWrapper<Response>) {
        auto results = response.getResults();
        if (results.data() == nullptr || results.size() != 1) {
            throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
        }
        return results[0];
    } else {
        if (response.results == nullptr || response.resultsSize != 1) {
            throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
        }
        return *response.results;
    }
}

}  // namespace opcua::services
