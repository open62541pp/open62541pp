#pragma once

#include <cassert>
#include <functional>  // invoke
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>  // forward

#include "open62541pp/async.hpp"
#include "open62541pp/client.hpp"
#include "open62541pp/detail/client_context.hpp"
#include "open62541pp/detail/exceptioncatcher.hpp"
#include "open62541pp/detail/open62541/client.h"
#include "open62541pp/detail/scope.hpp"
#include "open62541pp/exception.hpp"
#include "open62541pp/typeregistry.hpp"  // getDataType

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
        static_assert(std::is_invocable_v<CompletionHandler, TransformResult&>);
        using Context = std::tuple<ExceptionCatcher&, TransformResponse, CompletionHandler>;

        auto callback = [](UA_Client*, void* userdata, uint32_t /* reqId */, void* responsePtr) {
            assert(userdata != nullptr);
            std::unique_ptr<Context> context{static_cast<Context*>(userdata)};
            auto& catcher = std::get<ExceptionCatcher&>(*context);
            catcher.invoke([&] {
                if (responsePtr == nullptr) {
                    throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
                }
                Response& response = *static_cast<Response*>(responsePtr);
                auto result = std::invoke(std::get<TransformResponse>(*context), response);
                std::invoke(std::get<CompletionHandler>(*context), result);
            });
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
                // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks), false positive?
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
            if (opcua::detail::isBad(status)) {
                Response response{};
                response.responseHeader.serviceResult = status;
                callback(client.handle(), userdata, {}, &response);
            }
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
) noexcept(std::is_nothrow_invocable_v<TransformResponse, Response&>) {
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
