#pragma once

#include <cassert>
#include <functional>  // invoke
#include <memory>
#include <type_traits>
#include <utility>  // forward

#include "open62541pp/async.hpp"
#include "open62541pp/detail/client_utils.hpp"
#include "open62541pp/detail/exceptioncatcher.hpp"
#include "open62541pp/detail/open62541/client.h"
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

    template <typename CompletionHandler>
    static auto createCallbackAndContext(
        ExceptionCatcher& exceptionCatcher, CompletionHandler&& handler
    ) {
        static_assert(std::is_invocable_v<CompletionHandler, Response&>);

        struct Context {
            ExceptionCatcher* catcher;
            std::decay_t<CompletionHandler> handler;
        };

        const auto callback =
            [](UA_Client*, void* userdata, uint32_t /* reqId */, void* responsePtr) {
                std::unique_ptr<Context> context{static_cast<Context*>(userdata)};
                assert(context != nullptr);
                assert(context->catcher != nullptr);
                context->catcher->invoke([context = context.get(), responsePtr] {
                    if (responsePtr == nullptr) {
                        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
                    }
                    std::invoke(context->handler, *static_cast<Response*>(responsePtr));
                });
            };

        return CallbackAndContext<Context>{
            callback,
            std::make_unique<Context>(
                Context{&exceptionCatcher, std::forward<CompletionHandler>(handler)}
            )
        };
    }

    /**
     * Initiate open62541 async client operation with user-defined completion token.
     * @param client Instance of type Client
     * @param initiation Callable to initiate the async operation. Following signature is expected:
     *                   `void(UA_ClientAsyncServiceCallback callback, void* userdata)`
     * @param token Completion token
     */
    template <typename Initiation, typename CompletionToken>
    static auto initiate(Client& client, Initiation&& initiation, CompletionToken&& token) {
        static_assert(std::is_invocable_v<Initiation, UA_ClientAsyncServiceCallback, void*>);

        return asyncInitiate<Response>(
            [&](auto&& handler) {
                auto& catcher = opcua::detail::getExceptionCatcher(client);
                try {
                    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks), false positive?
                    auto callbackAndContext = createCallbackAndContext(
                        catcher, std::forward<decltype(handler)>(handler)
                    );
                    std::invoke(
                        std::forward<Initiation>(initiation),
                        callbackAndContext.callback,
                        callbackAndContext.context.get()
                    );
                    // initiation call might raise an exception
                    // transfer ownership to the callback afterwards
                    callbackAndContext.context.release();
                } catch (...) {
                    catcher.setException(std::current_exception());
                }
            },
            std::forward<CompletionToken>(token)
        );
    }
};

/// Async client service requests.
template <typename Request, typename Response, typename CompletionToken>
auto sendRequestAsync(Client& client, const Request& request, CompletionToken&& token) {
    return AsyncServiceAdapter<Response>::initiate(
        client,
        [&](UA_ClientAsyncServiceCallback callback, void* userdata) {
            throwIfBad(__UA_Client_AsyncService(
                opcua::detail::getHandle(client),
                &request,
                &getDataType<Request>(),
                callback,
                &getDataType<Response>(),
                userdata,
                nullptr
            ));
        },
        std::forward<CompletionToken>(token)
    );
}

/// Sync client service requests.
template <typename Request, typename Response>
Response sendRequest(Client& client, const Request& request) noexcept {
    Response response{};
    __UA_Client_Service(
        opcua::detail::getHandle(client),
        &request,
        &getDataType<Request>(),
        &response,
        &getDataType<Response>()
    );
    return response;
}

}  // namespace opcua::services::detail
