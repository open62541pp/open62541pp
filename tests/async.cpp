#include <doctest/doctest.h>

#include "open62541pp/Result.h"
#include "open62541pp/async.h"

using namespace opcua;

template <typename T, typename CompletionHandler>
static auto asyncTest(Result<T> result, CompletionHandler&& completionHandler) {
    return asyncInitiate<T>(
        [](auto... args) { std::invoke(args...); },
        std::forward<CompletionHandler>(completionHandler),
        result
    );
}

TEST_CASE("Async (callback completion token)") {
    SUBCASE("Void") {
        Result<void> result{UA_STATUSCODE_BADUNEXPECTEDERROR};
        std::optional<Result<void>> retrievedResult;
        asyncTest(result, [&](Result<void> res) { retrievedResult = res; });
        CHECK(retrievedResult.has_value());
        CHECK_EQ(retrievedResult->code(), result.code());  // NOLINT, false positive
    }

    SUBCASE("Value") {
        Result<int> result{11};
        std::optional<Result<int>> retrievedResult;
        asyncTest(result, [&](Result<int> res) { retrievedResult = res; });
        CHECK(retrievedResult.has_value());
        CHECK_EQ(retrievedResult->code(), result.code());  // NOLINT, false positive
        CHECK_EQ(retrievedResult->value(), result.value());  // NOLINT, false positive
    }

    SUBCASE("Error") {
        Result<int> result{BadResult{UA_STATUSCODE_BADUNEXPECTEDERROR}};
        std::optional<Result<int>> retrievedResult;
        asyncTest(result, [&](Result<int> res) { retrievedResult = res; });
        CHECK(retrievedResult.has_value());
        CHECK_EQ(retrievedResult->code(), result.code());  // NOLINT, false positive
    }
}

TEST_CASE("Async (future completion token)") {
    SUBCASE("Void") {
        Result<void> result{};
        std::future<void> future = asyncTest(result, useFuture);
        CHECK_NOTHROW(future.get());
    }

    SUBCASE("Result") {
        Result<double> result{11.11};
        std::future<double> future = asyncTest(result, useFuture);
        CHECK(future.get() == 11.11);
    }

    SUBCASE("Error") {
        Result<void> result{BadResult{UA_STATUSCODE_BADUNEXPECTEDERROR}};
        std::future<void> future = asyncTest(result, useFuture);
        CHECK_THROWS_WITH_AS(future.get(), "BadUnexpectedError", BadStatus);
    }
}

TEST_CASE("Async (deferred completion token)") {
    SUBCASE("Result") {
        Result<int> result{11};
        auto func = asyncTest(result, useDeferred);
        std::future<int> future = func(useFuture);
        CHECK(future.get() == 11);
        CHECK(func(useFuture).get() == 11);  // execute again
    }

    SUBCASE("Error") {
        Result<void> result{BadResult{UA_STATUSCODE_BADUNEXPECTEDERROR}};
        auto func = asyncTest(result, useDeferred);
        std::future<void> future = func(useFuture);
        CHECK_THROWS_WITH_AS(future.get(), "BadUnexpectedError", BadStatus);
    }
}

TEST_CASE("Async (detached completion token)") {
    SUBCASE("Result") {
        Result<int> result{11};
        CHECK_NOTHROW(asyncTest(result, useDetached));
    }

    SUBCASE("Error") {
        Result<int> result{BadResult{UA_STATUSCODE_BADUNEXPECTEDERROR}};
        CHECK_NOTHROW(asyncTest(result, useDetached));
    }
}
