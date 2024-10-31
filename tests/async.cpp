#include <functional>  // invoke
#include <future>
#include <utility>  // forward

#include <doctest/doctest.h>

#include "open62541pp/async.hpp"

using namespace opcua;

template <typename T, typename CompletionToken>
static auto asyncTest(T result, CompletionToken&& token) {
    return asyncInitiate<T>(
        [result](auto&& handler) mutable {
            std::invoke(std::forward<decltype(handler)>(handler), result);
        },
        std::forward<CompletionToken>(token)
    );
}

TEST_CASE("Async (callback completion token)") {
    int result{};
    asyncTest(5, [&](int value) { result = value; });
    CHECK(result == 5);
}

TEST_CASE("Async (future completion token)") {
    std::future<int> future = asyncTest(5, useFuture);
    CHECK(future.get() == 5);
}

TEST_CASE("Async (deferred completion token)") {
    auto func = asyncTest(5, useDeferred);
    auto future = func(useFuture);
    CHECK(future.get() == 5);
}

TEST_CASE("Async (detached completion token)") {
    CHECK_NOTHROW(asyncTest(5, useDetached));
}
