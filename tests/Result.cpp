#include <doctest/doctest.h>

#include <stdexcept>

#include "open62541pp/detail/Result.h"

using namespace opcua;

static constexpr StatusCode badCode(UA_STATUSCODE_BADUNEXPECTEDERROR);
static constexpr detail::BadResult badResult(badCode);

TEST_CASE("Result") {
    SUBCASE("code") {
        CHECK(detail::Result<int>(1).code() == UA_STATUSCODE_GOOD);
        CHECK(detail::Result<int>(badResult).code() == badCode);
    }

    SUBCASE("isGood") {
        CHECK(detail::Result<int>(1).isGood());
        CHECK_FALSE(detail::Result<int>(badResult).isGood());
    }

    SUBCASE("operator->") {
        struct S {
            int a;
        };

        detail::Result<S> result(S{1});
        CHECK(result->a == 1);
        CHECK(std::as_const(result)->a == 1);
    }

    SUBCASE("operator*") {
        detail::Result<int> result(1);
        CHECK(*result == 1);
        CHECK(*std::as_const(result) == 1);

        detail::Result<int> resultError(badResult);
        CHECK(*resultError == 0);
        CHECK(*std::as_const(resultError) == 0);

        SUBCASE("rvalue") {
            CHECK(*detail::Result<int>(1) == 1);
            CHECK(*detail::Result<int>(badResult) == 0);
        }
    }

    SUBCASE("isGood") {
        CHECK(detail::Result<int>(1).isGood());
        CHECK_FALSE(detail::Result<int>(badResult).isGood());
    }

    SUBCASE("value") {
        detail::Result<int> result(1);
        CHECK(result.value() == 1);
        CHECK(std::as_const(result).value() == 1);

        detail::Result<int> resultError(badResult);
        CHECK_THROWS_AS(resultError.value(), BadStatus);
        CHECK_THROWS_AS(std::as_const(resultError).value(), BadStatus);

        SUBCASE("rvalue") {
            CHECK(detail::Result<int>(1).value() == 1);
            CHECK_THROWS_AS(detail::Result<int>(badResult).value(), BadStatus);
        }
    }

    SUBCASE("valueOr") {
        detail::Result<int> result(1);
        CHECK(result.valueOr(2) == 1);

        detail::Result<int> resultError(badResult);
        CHECK(resultError.valueOr(2) == 2);

        SUBCASE("rvalue") {
            CHECK(detail::Result<int>(1).valueOr(2) == 1);
            CHECK(detail::Result<int>(badResult).valueOr(2) == 2);
        }
    }
}

TEST_CASE("Result (void template specialization)") {
    SUBCASE("code") {
        CHECK(detail::Result<void>().code() == UA_STATUSCODE_GOOD);
        CHECK(detail::Result<void>(badResult).code() == badCode);
    }

    SUBCASE("isGood") {
        CHECK(detail::Result<void>().isGood());
        CHECK_FALSE(detail::Result<void>(badResult).isGood());
    }

    SUBCASE("value") {
        CHECK_NOTHROW(detail::Result<void>().value());
        CHECK_THROWS_AS(detail::Result<void>(badResult).value(), BadStatus);
    }
}

TEST_CASE("tryInvoke") {
    SUBCASE("Result") {
        auto result = detail::tryInvoke([] { return 1; });
        CHECK(result.isGood());
        CHECK(result.value() == 1);
        CHECK(result.code() == 0);
    }

    SUBCASE("BadStatus exception") {
        auto result = detail::tryInvoke([] { throw BadStatus(badCode); });
        CHECK_FALSE(result.isGood());
        CHECK(result.code() == badCode);
    }

    SUBCASE("Other exception types") {
        auto result = detail::tryInvoke([] { throw std::runtime_error("test"); });
        CHECK_FALSE(result.isGood());
        CHECK(result.code() == UA_STATUSCODE_BADINTERNALERROR);
    }
}

TEST_CASE("tryInvokeGetStatus") {
    CHECK(detail::tryInvokeGetStatus([]() -> void { return; }) == UA_STATUSCODE_GOOD);
    CHECK(detail::tryInvokeGetStatus([]() -> void { throw BadStatus(badCode); }) == badCode);
    CHECK(detail::tryInvokeGetStatus([]() -> StatusCode { return 1; }) == 1);
    CHECK(detail::tryInvokeGetStatus([]() -> StatusCode { throw BadStatus(badCode); }) == badCode);
}
