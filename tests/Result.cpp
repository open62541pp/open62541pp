#include <doctest/doctest.h>

#include <stdexcept>

#include "open62541pp/Result.h"
#include "open62541pp/detail/result_util.h"

using namespace opcua;

static constexpr StatusCode badCode(UA_STATUSCODE_BADUNEXPECTEDERROR);
static constexpr BadResult badResult(badCode);

TEST_CASE("Result") {
    SUBCASE("operator->") {
        struct S {
            int a;
        };

        Result<S> result(S{1});
        CHECK(result->a == 1);
        CHECK(std::as_const(result)->a == 1);
    }

    SUBCASE("operator*") {
        Result<int> result(1);
        CHECK(*result == 1);
        CHECK(*std::as_const(result) == 1);

        SUBCASE("rvalue") {
            CHECK(*Result<int>(1) == 1);
        }
    }

    SUBCASE("code") {
        CHECK(Result<int>(1).code() == UA_STATUSCODE_GOOD);
        CHECK(Result<int>(badResult).code() == badCode);
    }

    SUBCASE("hasValue") {
        Result<int> result(1);
        CHECK(result.hasValue());

        Result<int> resultError(badResult);
        CHECK_FALSE(resultError.hasValue());
    }

    SUBCASE("value") {
        Result<int> resultDefault;
        CHECK(resultDefault.value() == 0);

        Result<int> result(1);
        CHECK(result.value() == 1);
        CHECK(std::as_const(result).value() == 1);

        Result<int> resultError(badResult);
        CHECK_THROWS_AS(resultError.value(), BadStatus);
        CHECK_THROWS_AS(std::as_const(resultError).value(), BadStatus);

        SUBCASE("rvalue") {
            CHECK(Result<int>(1).value() == 1);
            CHECK_THROWS_AS(Result<int>(badResult).value(), BadStatus);
        }
    }

    SUBCASE("valueOr") {
        Result<int> result(1);
        CHECK(result.valueOr(2) == 1);

        Result<int> resultError(badResult);
        CHECK(resultError.valueOr(2) == 2);

        SUBCASE("rvalue") {
            CHECK(Result<int>(1).valueOr(2) == 1);
            CHECK(Result<int>(badResult).valueOr(2) == 2);
        }
    }
}

TEST_CASE("Result (void template specialization)") {
    SUBCASE("operator StatusCode") {
        CHECK(static_cast<StatusCode>(Result<void>()) == UA_STATUSCODE_GOOD);
        CHECK(static_cast<StatusCode>(Result<void>(badResult)) == UA_STATUSCODE_BADUNEXPECTEDERROR);
    }

    SUBCASE("code") {
        CHECK(Result<void>().code() == UA_STATUSCODE_GOOD);
        CHECK(Result<void>(badResult).code() == badCode);
    }

    SUBCASE("hasValue") {
        Result<void> result{};
        CHECK(result.hasValue());

        Result<void> resultError(badResult);
        CHECK_FALSE(resultError.hasValue());
    }

    SUBCASE("value") {
        Result<void> result{};
        CHECK_NOTHROW(result.value());

        Result<void> resultError(badResult);
        CHECK_THROWS_AS(resultError.value(), BadStatus);

        SUBCASE("rvalue") {
            CHECK_NOTHROW(Result<void>().value());
            CHECK_THROWS_AS(Result<void>(badResult).value(), BadStatus);
        }
    }
}

TEST_CASE("tryInvoke") {
    SUBCASE("Result<int>") {
        auto result = detail::tryInvoke([] { return 1; });
        CHECK(result.code() == UA_STATUSCODE_GOOD);
        CHECK(result.value() == 1);
    }

    SUBCASE("Result<void>") {
        auto result = detail::tryInvoke([] { return; });
        CHECK(result.code() == UA_STATUSCODE_GOOD);
    }

    SUBCASE("BadStatus exception") {
        auto result = detail::tryInvoke([] { throw BadStatus(badCode); });
        CHECK(result.code() == badCode);
    }

    SUBCASE("Other exception types") {
        auto result = detail::tryInvoke([] { throw std::runtime_error("test"); });
        CHECK(result.code() == UA_STATUSCODE_BADINTERNALERROR);
    }
}
