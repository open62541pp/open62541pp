#include <string>

#include <doctest/doctest.h>

#include "open62541pp/services/detail/async_transform.hpp"

using namespace opcua;

template <typename T, typename CompletionHandler>
static auto asyncTest(T result, CompletionHandler&& completionHandler) {
    return asyncInitiate<T>(
        [](auto&& handler, T result) {
            std::invoke(std::forward<decltype(handler)>(handler), result);
        },
        std::forward<CompletionHandler>(completionHandler),
        result
    );
}

TEST_CASE("TransformToken") {
    SUBCASE("Callback") {
        std::string result;
        asyncTest(
            5,
            services::detail::TransformToken(
                [](int value) { return std::to_string(value); },
                [&](const std::string& str) { result = str; }
            )
        );
        CHECK(result == "5");
    }

    SUBCASE("Future") {
        std::future<std::string> future = asyncTest(
            5,
            services::detail::TransformToken(
                [](int value) { return std::to_string(value); }, useFuture
            )
        );
        CHECK(future.get() == "5");
    }
}
