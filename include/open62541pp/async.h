#pragma once

#include <functional>  // invoke
#include <future>
#include <tuple>
#include <type_traits>
#include <utility>  // forward, move

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/types/Builtin.h"  // StatusCode

namespace opcua {

template <class CompletionToken, typename Result>
struct AsyncResult {
    template <typename Initiation, typename CompletionHandler, typename... Args>
    static void initiate(Initiation&& initiation, CompletionHandler&& handler, Args&&... args) {
        if constexpr (std::is_void_v<Result>) {
            static_assert(std::is_invocable_v<CompletionHandler, StatusCode>);
        } else {
            static_assert(std::is_invocable_v<CompletionHandler, StatusCode, Result>);
        }
        std::invoke(
            std::forward<Initiation>(initiation),
            std::forward<CompletionHandler>(handler),
            std::forward<Args>(args)...
        );
    }
};

template <typename Result, typename Initiation, typename CompletionToken, typename... Args>
inline auto asyncInitiate(Initiation&& initiation, CompletionToken&& token, Args&&... args) {
    return AsyncResult<std::decay_t<CompletionToken>, Result>::initiate(
        std::forward<Initiation>(initiation),
        std::forward<CompletionToken>(token),
        std::forward<Args>(args)...
    );
}

/* ------------------------------------------- Future ------------------------------------------- */

struct UseFutureToken {};

constexpr UseFutureToken useFuture;

template <typename Result>
struct AsyncResult<UseFutureToken, Result> {
    template <typename Initiation, typename... Args>
    static auto initiate(Initiation&& initiation, UseFutureToken /* unused */, Args&&... args) {
        std::promise<Result> promise;
        auto future = promise.get_future();
        std::invoke(
            std::forward<Initiation>(initiation),
            [p = std::move(promise)](StatusCode code, auto&&... result) mutable {
                if (code.isBad()) {
                    p.set_exception(std::make_exception_ptr(BadStatus(code)));
                } else {
                    p.set_value(std::forward<decltype(result)>(result)...);
                }
            },
            std::forward<Args>(args)...
        );
        return future;
    }
};

/* ------------------------------------------ Deferred ------------------------------------------ */

struct UseDeferredToken {};

constexpr UseDeferredToken useDeferred{};

template <typename Result>
struct AsyncResult<UseDeferredToken, Result> {
    template <typename Initiation, typename... Args>
    static auto initiate(Initiation&& initiation, UseDeferredToken /* unused */, Args&&... args) {
        return [initiation = std::forward<Initiation>(initiation),
                argsPack = std::make_tuple(std::forward<Args>(args)...)](auto&& token) mutable {
            return std::apply(
                [&](auto&&... argsInner) {
                    return AsyncResult<std::decay_t<decltype(token)>, Result>::initiate(
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

}  // namespace opcua
