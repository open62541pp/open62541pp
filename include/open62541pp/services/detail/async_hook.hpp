#pragma once

#include <functional>  // invoke
#include <type_traits>
#include <utility>

#include "open62541pp/async.hpp"

namespace opcua::services::detail {

/**
 * Special token to execute a hook function with the const result within the completion handler.
 * The HookToken wraps a hook function (object) and the original completion token.
 */
template <typename HookFunction, typename CompletionToken>
struct HookToken {
    HookToken(HookFunction&& hookFunction, CompletionToken&& completionToken)
        : hook(std::move(hookFunction)),
          token(std::move(completionToken)) {}

    std::decay_t<HookFunction> hook;
    std::decay_t<CompletionToken> token;
};

template <typename HookFunction, typename CompletionToken>
HookToken(HookFunction&&, CompletionToken&&) -> HookToken<HookFunction, CompletionToken>;

}  // namespace opcua::services::detail

namespace opcua {

template <typename HookFunction, typename CompletionToken, typename T>
struct AsyncResult<services::detail::HookToken<HookFunction, CompletionToken>, T> {
    using Token = services::detail::HookToken<HookFunction, CompletionToken>;

    template <typename Initiation, typename... Args>
    static auto initiate(
        Initiation&& initiation,
        Token&& token,  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
        Args&&... args
    ) {
        return asyncInitiate<T>(
            [innerInitiation = std::forward<Initiation>(initiation),
             innerHook = std::forward<Token>(token).hook](
                auto&& handler, auto&&... innerArgs
            ) mutable {
                std::invoke(
                    innerInitiation,
                    [innerHook = std::move(innerHook),
                     innerHandler = std::forward<decltype(handler)>(handler)](auto&& result
                    ) mutable {
                        std::invoke(innerHook, std::as_const(result));
                        std::invoke(innerHandler, result);
                    },
                    std::forward<decltype(innerArgs)>(innerArgs)...
                );
            },
            std::forward<Token>(token).token,
            std::forward<Args>(args)...
        );
    }
};

}  // namespace opcua
