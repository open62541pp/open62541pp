#pragma once

#include <functional>  // invoke
#include <future>
#include <tuple>
#include <type_traits>
#include <utility>  // forward, move

#include "open62541pp/types.hpp"  // StatusCode

namespace opcua {

/**
 * @defgroup Async Asynchronous operations
 * The asynchronous model is based on (Boost) Asio's universal model for asynchronous operations.
 * Each async function takes a `CompletionToken` as it's last parameter.
 * The completion token can be a callable with the signature `void(T)` or `void(T&)` where `T` is a
 * function-specific result type.
 *
 * @see https://think-async.com/asio/asio-1.28.0/doc/asio/overview/model/async_ops.html
 * @see https://think-async.com/asio/asio-1.28.0/doc/asio/overview/model/completion_tokens.html
 * @see https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3747.pdf
 * @{
 */

template <class CompletionToken, typename T>
struct AsyncResult {
    template <typename Initiation, typename CompletionHandler, typename... Args>
    static void initiate(Initiation&& initiation, CompletionHandler&& handler, Args&&... args) {
        static_assert(std::is_invocable_v<CompletionHandler, T> || std::is_invocable_v<CompletionHandler, T&>);
        std::invoke(
            std::forward<Initiation>(initiation),
            std::forward<CompletionHandler>(handler),
            std::forward<Args>(args)...
        );
    }
};

template <typename T, typename Initiation, typename CompletionToken, typename... Args>
auto asyncInitiate(Initiation&& initiation, CompletionToken&& token, Args&&... args) {
    return AsyncResult<std::decay_t<CompletionToken>, T>::initiate(
        std::forward<Initiation>(initiation),
        std::forward<CompletionToken>(token),
        std::forward<Args>(args)...
    );
}

/* ------------------------------------------- Future ------------------------------------------- */

/**
 * Future completion token type.
 * A completion token that causes an asynchronous operation to return a future.
 */
struct UseFutureToken {};

/**
 * Future completion token object.
 * @see UseFutureToken
 */
constexpr UseFutureToken useFuture;

template <typename T>
struct AsyncResult<UseFutureToken, T> {
    template <typename Initiation, typename... Args>
    static auto initiate(Initiation&& initiation, UseFutureToken /*unused*/, Args&&... args) {
        std::promise<T> promise;
        auto future = promise.get_future();
        std::invoke(
            std::forward<Initiation>(initiation),
            [p = std::move(promise)](T& result) mutable { p.set_value(std::move(result)); },
            std::forward<Args>(args)...
        );
        return future;
    }
};

/* ------------------------------------------ Deferred ------------------------------------------ */

/**
 * Deferred completion token type.
 * The token is used to indicate that an asynchronous operation should return a function object to
 * lazily launch the operation.
 */
struct UseDeferredToken {};

/**
 * Deferred completion token object.
 * @see UseDeferredToken
 */
constexpr UseDeferredToken useDeferred;

template <typename T>
struct AsyncResult<UseDeferredToken, T> {
    template <typename Initiation, typename... Args>
    static auto initiate(Initiation&& initiation, UseDeferredToken /*unused*/, Args&&... args) {
        return [initiation = std::forward<Initiation>(initiation),
                argsPack = std::make_tuple(std::forward<Args>(args)...)](auto&& token) mutable {
            return std::apply(
                [&](auto&&... argsInner) {
                    return AsyncResult<std::decay_t<decltype(token)>, T>::initiate(
                        std::move(initiation),
                        std::forward<decltype(token)>(token),
                        std::forward<decltype(argsInner)>(argsInner)...
                    );
                },
                std::move(argsPack)
            );
        };
    }
};

/* ------------------------------------------ Detached ------------------------------------------ */

/**
 * Detached completion token type.
 * The token is used to indicate that an asynchronous operation is detached. That is, there is no
 * completion handler waiting for the operation's result.
 */
struct UseDetachedToken {};

/**
 * Detached completion token object.
 * @see UseDetachedToken
 */
constexpr UseDetachedToken useDetached;

template <typename T>
struct AsyncResult<UseDetachedToken, T> {
    template <typename Initiation, typename... Args>
    static auto initiate(Initiation&& initiation, UseDetachedToken /*unused*/, Args&&... args) {
        std::invoke(
            std::forward<Initiation>(initiation),
            [](auto&&...) {
                // ...
            },
            std::forward<Args>(args)...
        );
    }
};

/* ------------------------------------------ Defaults ------------------------------------------ */

/**
 * Default completion token for async operations.
 * @see UseFutureToken
 */
using DefaultCompletionToken = UseFutureToken;

/**
 * @}
 */

}  // namespace opcua
