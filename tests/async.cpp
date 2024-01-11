#include <doctest/doctest.h>

#include "open62541pp/async.h"

using namespace opcua;

template <typename CompletionHandler>
static auto asyncTest(StatusCode code, CompletionHandler&& completionHandler) {
    return asyncInitiate<void>(
        [](auto&& handler, StatusCode code_) {
            INFO("Init async operation (void result) with code ", code_);
            std::invoke(handler, code_);
        },
        std::forward<CompletionHandler>(completionHandler),
        code
    );
}

template <typename T, typename CompletionHandler>
static auto asyncTest(StatusCode code, T value, CompletionHandler&& completionHandler) {
    return asyncInitiate<T>(
        [](auto&& handler, StatusCode code_, T value_) {
            INFO("Init async operation with code ", code_, " and value ", value_);
            std::invoke(handler, code_, value_);
        },
        std::forward<CompletionHandler>(completionHandler),
        code,
        value
    );
}

TEST_CASE("Async (callback completion token)") {
    StatusCode code{};
    int value{};

    SUBCASE("Void") {
        asyncTest(UA_STATUSCODE_BADUNEXPECTEDERROR, [&](StatusCode code_) { code = code_; });
        CHECK(code == UA_STATUSCODE_BADUNEXPECTEDERROR);
    }

    SUBCASE("Result") {
        asyncTest(UA_STATUSCODE_BADUNEXPECTEDERROR, 11, [&](StatusCode code_, int value_) {
            code = code_;
            value = value_;
        });
        CHECK(code == UA_STATUSCODE_BADUNEXPECTEDERROR);
        CHECK(value == 11);
    }
}

TEST_CASE("Async (future completion token)") {
    SUBCASE("Void") {
        std::future<void> future = asyncTest(UA_STATUSCODE_GOOD, useFuture);
        CHECK_NOTHROW(future.get());
    }

    SUBCASE("Result") {
        std::future<double> future = asyncTest(UA_STATUSCODE_GOOD, 11.11, useFuture);
        CHECK(future.get() == 11.11);
    }

    SUBCASE("Error") {
        std::future<void> future = asyncTest(UA_STATUSCODE_BADUNEXPECTEDERROR, useFuture);
        CHECK_THROWS_WITH_AS(future.get(), "BadUnexpectedError", BadStatus);
    }
}

TEST_CASE("Async (deferred completion token)") {
    SUBCASE("Result") {
        auto func = asyncTest(UA_STATUSCODE_GOOD, 11, useDeferred);
        std::future<int> future = func(useFuture);
        CHECK(future.get() == 11);
        CHECK(func(useFuture).get() == 11);  // execute again
    }

    SUBCASE("Error") {
        auto func = asyncTest(UA_STATUSCODE_BADUNEXPECTEDERROR, useDeferred);
        std::future<void> future = func(useFuture);
        CHECK_THROWS_WITH_AS(future.get(), "BadUnexpectedError", BadStatus);
    }
}
