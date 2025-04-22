#pragma once

#include <functional>  // invoke
#include <type_traits>  // invoke_result_t

#include "open62541pp/result.hpp"

namespace opcua::detail {

// Derive Result type from T
template <typename T>
struct ResultType {
    using Type = opcua::Result<T>;
};

template <>
struct ResultType<StatusCode> {
    using Type = opcua::Result<void>;
};

template <>
struct ResultType<BadResult> {
    using Type = opcua::Result<void>;
};

/**
 * Invoke a function and capture its Result (value or status code).
 * This is especially useful for C-API callbacks, that are executed within the open62541 event loop.
 */
template <typename F, typename... Args>
auto tryInvoke(F&& func, Args&&... args) noexcept ->
    typename ResultType<std::invoke_result_t<F, Args...>>::Type {
    using ReturnType = std::invoke_result_t<F, Args...>;
    try {
        if constexpr (std::is_void_v<ReturnType>) {
            std::invoke(std::forward<F>(func), std::forward<Args>(args)...);
            return {};
        } else {
            return std::invoke(std::forward<F>(func), std::forward<Args>(args)...);
        }
    } catch (...) {
        return BadResult{getStatusCode(std::current_exception())};
    }
}

}  // namespace opcua::detail
