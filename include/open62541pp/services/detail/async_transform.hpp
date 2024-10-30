#pragma once

#include <functional>  // invoke
#include <type_traits>
#include <utility>

#include "open62541pp/async.hpp"

namespace opcua::services::detail {

template <typename CompletionHandler, typename Transform>
constexpr auto asyncTransform(CompletionHandler&& handler, Transform&& transform) {
    return [innerHandler = std::forward<CompletionHandler>(handler),
            innerTransform = std::forward<Transform>(transform)](auto&& result) mutable {
        auto transformed = std::invoke(innerTransform, std::forward<decltype(result)>(result));
        std::invoke(innerHandler, transformed);
    };
}

/**
 * Special token to transform async results within the completion handler.
 * The TransformToken wraps a transform function (object) and the original completion token.
 */
template <typename TransformFunction, typename CompletionToken>
struct TransformToken {
    constexpr TransformToken(TransformFunction&& transform, CompletionToken&& token)
        : transform(std::move(transform)),
          token(std::move(token)) {}

    std::decay_t<TransformFunction> transform;
    std::decay_t<CompletionToken> token;
};

template <typename TransformFunction, typename CompletionToken>
TransformToken(TransformFunction&&, CompletionToken&&)
    -> TransformToken<TransformFunction, CompletionToken>;

}  // namespace opcua::services::detail

namespace opcua {

template <typename TransformFunction, typename CompletionToken, typename T>
struct AsyncResult<services::detail::TransformToken<TransformFunction, CompletionToken>, T> {
    using Token = services::detail::TransformToken<TransformFunction, CompletionToken>;

    template <typename Initiation, typename... Args>
    static auto initiate(
        Initiation&& initiation,
        Token&& token,  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
        Args&&... args
    ) {
        using TransformResult = std::invoke_result_t<TransformFunction, T>;
        return asyncInitiate<TransformResult>(
            [](auto&& handler, auto&& initiation, auto&& transform, auto&&... args) {
                std::invoke(
                    std::forward<decltype(initiation)>(initiation),
                    services::detail::asyncTransform(
                        std::forward<decltype(handler)>(handler),
                        std::forward<decltype(transform)>(transform)
                    ),
                    std::forward<decltype(args)>(args)...
                );
            },
            std::forward<Token>(token).token,
            std::forward<Initiation>(initiation),
            std::forward<Token>(token).transform,
            std::forward<Args>(args)...
        );
    }
};

}  // namespace opcua
