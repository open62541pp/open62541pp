#pragma once

#include <cassert>
#include <tuple>
#include <type_traits>
#include <utility>  // exchange, forward, move

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/TypeRegistry.h"
#include "open62541pp/async.h"
#include "open62541pp/detail/Result.h"
#include "open62541pp/detail/ScopeExit.h"
#include "open62541pp/open62541.h"

namespace opcua::detail {

struct MoveResponse {
    template <typename Response>
    [[nodiscard]] constexpr auto operator()(Response&& value) noexcept {
        return std::exchange(std::forward<Response>(value), {});
    }
};

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
                auto result = tryInvoke(std::get<TransformResponse>(*context), response);
                if constexpr (std::is_void_v<Result>) {
                    std::invoke(handler, result.code());
                } else {
                    std::invoke(handler, result.code(), *result);
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
            &getDataType<Request>(),
            callback,
            &getDataType<Response>(),
            context.release(),  // userdata, transfer ownership to callback
            nullptr
        );
        throwOnBadStatus(status);
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
template <typename Request, typename Response, typename TransformResponse>
static auto sendRequest(
    Client& client,
    const Request& request,
    TransformResponse&& transformResponse,
    SyncOperation /*unused*/
) {
    Response response{};
    const auto responseDeleter = ScopeExit([&] { clear(response, getDataType<Response>()); });

    __UA_Client_Service(
        client.handle(), &request, &getDataType<Request>(), &response, &getDataType<Response>()
    );

    return std::invoke(std::forward<TransformResponse>(transformResponse), response);
}

}  // namespace opcua::detail
