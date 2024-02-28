#pragma once

#include <cassert>
#include <functional>  // invoke
#include <tuple>
#include <type_traits>
#include <utility>  // forward

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/TypeRegistry.h"
#include "open62541pp/async.h"
#include "open62541pp/detail/ClientContext.h"
#include "open62541pp/detail/ExceptionCatcher.h"
#include "open62541pp/detail/ScopeExit.h"
#include "open62541pp/detail/result_util.h"
#include "open62541pp/open62541.h"

namespace opcua::services::detail {

/**
 * Adapter to initiate open62541 async client operations with completion tokens.
 */
template <typename Response>
struct AsyncServiceAdapter {
    using ExceptionCatcher = opcua::detail::ExceptionCatcher;

    template <typename Context>
    struct CallbackAndContext {
        UA_ClientAsyncServiceCallback callback;
        std::unique_ptr<Context> context;
    };

    template <typename CompletionHandler, typename TransformResponse>
    static auto createCallbackAndContext(
        ExceptionCatcher& exceptionCatcher,
        TransformResponse&& transformResponse,
        CompletionHandler&& completionHandler
    ) {
        static_assert(std::is_invocable_v<TransformResponse, Response&>);
        using TransformResult = std::invoke_result_t<TransformResponse, Response&>;
        using Context = std::tuple<ExceptionCatcher&, TransformResponse, CompletionHandler>;

        auto callback = [](UA_Client*, void* userdata, uint32_t /* reqId */, void* responsePtr) {
            assert(userdata != nullptr);
            std::unique_ptr<Context> context{static_cast<Context*>(userdata)};
            auto& catcher = std::get<ExceptionCatcher&>(*context);
            auto& handler = std::get<CompletionHandler>(*context);

            auto result = [&]() -> opcua::Result<TransformResult> {
                if (responsePtr == nullptr) {
                    return opcua::BadResult(UA_STATUSCODE_BADUNEXPECTEDERROR);
                }
                Response& response = *static_cast<Response*>(responsePtr);
                return opcua::detail::tryInvoke(std::get<TransformResponse>(*context), response);
            }();

            if constexpr (std::is_void_v<TransformResult>) {
                catcher.invoke(handler, result.code());
            } else {
                catcher.invoke(handler, result.code(), *result);
            }
        };

        return CallbackAndContext<Context>{
            callback,
            std::make_unique<Context>(
                exceptionCatcher,
                std::forward<TransformResponse>(transformResponse),
                std::forward<CompletionHandler>(completionHandler)
            )
        };
    }

    /**
     * Initiate open62541 async client operation with user-defined completion token.
     * @param client Instance of type Client
     * @param initiation Callable to initiate the async operation. Following signature is expected:
     *                   `void(UA_ClientAsyncServiceCallback callback, void* userdata)`
     * @param transformResponse Callable to transform the `Response` type to the desired output
     * @param token Completion token
     */
    template <typename Initiation, typename TransformResponse, typename CompletionToken>
    static auto initiate(
        Client& client,
        Initiation&& initiation,
        TransformResponse&& transformResponse,
        CompletionToken&& token
    ) {
        static_assert(std::is_invocable_v<Initiation, UA_ClientAsyncServiceCallback, void*>);
        using TransformResult = std::invoke_result_t<TransformResponse, Response&>;

        return asyncInitiate<TransformResult>(
            [&](auto&& completionHandler, auto&& transform) {
                // NOLINTNEXTLINE, false positive?
                auto callbackAndContext = createCallbackAndContext(
                    opcua::detail::getContext(client).exceptionCatcher,
                    std::forward<decltype(transform)>(transform),
                    std::forward<decltype(completionHandler)>(completionHandler)
                );

                std::invoke(
                    std::forward<Initiation>(initiation),
                    callbackAndContext.callback,
                    callbackAndContext.context.release()  // transfer ownership to callback
                );
            },
            std::forward<CompletionToken>(token),
            std::forward<TransformResponse>(transformResponse)
        );
    }
};

template <typename Request, typename Response, typename TransformResponse, typename CompletionToken>
auto sendRequest(
    Client& client,
    const Request& request,
    TransformResponse&& transformResponse,
    CompletionToken&& token
) {
    return AsyncServiceAdapter<Response>::initiate(
        client,
        [&](UA_ClientAsyncServiceCallback callback, void* userdata) {
            const auto status = __UA_Client_AsyncService(
                client.handle(),
                &request,
                &getDataType<Request>(),
                callback,
                &getDataType<Response>(),
                userdata,
                nullptr
            );
            throwIfBad(status);
        },
        std::forward<TransformResponse>(transformResponse),
        std::forward<CompletionToken>(token)
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
    const auto responseDeleter = opcua::detail::ScopeExit([&] {
        opcua::detail::clear(response, getDataType<Response>());
    });

    __UA_Client_Service(
        client.handle(), &request, &getDataType<Request>(), &response, &getDataType<Response>()
    );

    return std::invoke(std::forward<TransformResponse>(transformResponse), response);
}

}  // namespace opcua::services::detail
