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
#include "open62541pp/ua/types.hpp"

namespace opcua::services::detail {

/**
 * Adapter to initiate open62541 async client operations with completion tokens.
 */
template <typename Response, bool TypedCallback>
struct ResponseArgSelector {
    using type = void*;
};

template <typename Response>
struct ResponseArgSelector<Response, true> {
    using type = typename Response::NativeType*;
};

template <typename Response, bool TypedCallback = false>
struct AsyncServiceAdapter {
    using ExceptionCatcher = opcua::detail::ExceptionCatcher;
    static constexpr bool typedCallback = TypedCallback && UAPP_OPEN62541_VER_GE(1, 5);
    using ResponseArg = typename ResponseArgSelector<Response, typedCallback>::type;
    using Callback = void (*)(UA_Client*, void*, uint32_t, ResponseArg);

    template <typename Context>
    struct CallbackAndContext {
        Callback callback;
        std::unique_ptr<Context> context;
    };

    template <typename CompletionHandler>
    static auto makeCallbackAndContext(
        ExceptionCatcher& exceptionCatcher, CompletionHandler&& handler
    ) {
        static_assert(std::is_invocable_v<CompletionHandler, Response&>);

        struct Context {
            ExceptionCatcher* catcher;
            std::decay_t<CompletionHandler> handler;
        };

        const Callback callback =
            [](UA_Client*, void* userdata, uint32_t /* reqId */, ResponseArg responsePtr) {
                std::unique_ptr<Context> context{static_cast<Context*>(userdata)};
                assert(context != nullptr);
                assert(context->catcher != nullptr);
                context->catcher->invoke([context = context.get(), responsePtr] {
                    void* raw = responsePtr;  // valid whether responsePtr is void* or typed
                    if (raw == nullptr) {
                        throw BadStatus{UA_STATUSCODE_BADUNEXPECTEDERROR};
                    }
                    std::invoke(context->handler, *static_cast<Response*>(raw));
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
     *                   ```
     *                   void(
     *                       UA_ClientAsyncServiceCallback callback,
     *                       void* userdata,
     *                       Args... args)
     *                   ```
     * @param token Completion token
     */
    template <typename Initiation, typename CompletionToken, typename... Args>
    static auto initiate(
        Client& client, Initiation&& initiation, CompletionToken&& token, Args&&... args
    ) {
        static_assert(std::is_invocable_v<Initiation, Callback, void*, Args&&...>);

        return asyncInitiate<Response>(
            [&client](auto&& handler, auto&& innerInitiation, auto&&... innerArgs) {
                auto& catcher = opcua::detail::getExceptionCatcher(client);
                try {
                    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks), false positive?
                    auto callbackAndContext = makeCallbackAndContext(
                        catcher, std::forward<decltype(handler)>(handler)
                    );
                    std::invoke(
                        std::forward<decltype(innerInitiation)>(innerInitiation),
                        callbackAndContext.callback,
                        callbackAndContext.context.get(),
                        std::forward<decltype(innerArgs)>(innerArgs)...
                    );
                    // initiation call might raise an exception
                    // transfer ownership to the callback afterwards
                    callbackAndContext.context.release();
                } catch (...) {
                    catcher.setException(std::current_exception());
                }
            },
            std::forward<CompletionToken>(token),
            std::forward<Initiation>(initiation),
            std::forward<Args>(args)...
        );
    }
};

/// Async client service requests.
template <typename Request, typename Response, typename CompletionToken>
auto sendRequestAsync(Client& client, const Request& request, CompletionToken&& token) {
    return AsyncServiceAdapter<Response>::initiate(
        client,
        [&client](
            UA_ClientAsyncServiceCallback callback, void* userdata, const Request& innerRequest
        ) {
            throwIfBad(__UA_Client_AsyncService(
                opcua::detail::getHandle(client),
                &innerRequest,
                &getDataType<Request>(),
                callback,
                &getDataType<Response>(),
                userdata,
                nullptr
            ));
        },
        std::forward<CompletionToken>(token),
        request
    );
}

#if UAPP_OPEN62541_VER_GE(1, 5)

// the generic UA_Client_Service was removed in v1.5, use the typed service functions
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UAPP_CLIENT_SERVICE(service, Type)                                                         \
    inline UA_##Type##Response callService(                                                        \
        UA_Client* client, const UA_##Type##Request& request                                       \
    ) noexcept {                                                                                   \
        return UA_Client_Service_##service(client, request);                                       \
    }

UAPP_CLIENT_SERVICE(addNodes, AddNodes)
UAPP_CLIENT_SERVICE(addReferences, AddReferences)
UAPP_CLIENT_SERVICE(deleteNodes, DeleteNodes)
UAPP_CLIENT_SERVICE(deleteReferences, DeleteReferences)
UAPP_CLIENT_SERVICE(browse, Browse)
UAPP_CLIENT_SERVICE(browseNext, BrowseNext)
UAPP_CLIENT_SERVICE(translateBrowsePathsToNodeIds, TranslateBrowsePathsToNodeIds)
UAPP_CLIENT_SERVICE(registerNodes, RegisterNodes)
UAPP_CLIENT_SERVICE(unregisterNodes, UnregisterNodes)
UAPP_CLIENT_SERVICE(read, Read)
UAPP_CLIENT_SERVICE(write, Write)
UAPP_CLIENT_SERVICE(call, Call)
#ifdef UA_ENABLE_QUERY
UAPP_CLIENT_SERVICE(queryFirst, QueryFirst)
UAPP_CLIENT_SERVICE(queryNext, QueryNext)
#endif
#ifdef UA_ENABLE_HISTORIZING
UAPP_CLIENT_SERVICE(historyRead, HistoryRead)
UAPP_CLIENT_SERVICE(historyUpdate, HistoryUpdate)
#endif

#undef UAPP_CLIENT_SERVICE

#endif

/// Sync client service requests.
template <typename Request, typename Response>
Response sendRequest(Client& client, const Request& request) noexcept {
#if UAPP_OPEN62541_VER_GE(1, 5)
    return Response(callService(opcua::detail::getHandle(client), request));
#else
    Response response{};
    __UA_Client_Service(
        opcua::detail::getHandle(client),
        &request,
        &getDataType<Request>(),
        &response,
        &getDataType<Response>()
    );
    return response;
#endif
}

}  // namespace opcua::services::detail
