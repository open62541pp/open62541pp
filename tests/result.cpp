#include <stdexcept>
#include <utility>  // as_const

#include <catch2/catch_test_macros.hpp>

#include "open62541pp/detail/result_utils.hpp"
#include "open62541pp/result.hpp"

using namespace opcua;

static constexpr StatusCode badCode(UA_STATUSCODE_BADUNEXPECTEDERROR);
static constexpr BadResult badResult(badCode);

TEST_CASE("Result") {
    SECTION("Constructors") {
        CHECK(Result<int>().value() == 0);
        CHECK(Result<int>(1).value() == 1);
        CHECK(Result<int>(1, UA_STATUSCODE_GOODCLAMPED).value() == 1);
        CHECK(Result<int>(1, UA_STATUSCODE_GOODCLAMPED).code() == UA_STATUSCODE_GOODCLAMPED);
        CHECK(
            Result<int>(BadResult(UA_STATUSCODE_BADINTERNALERROR)).code() ==
            UA_STATUSCODE_BADINTERNALERROR
        );
    }

    SECTION("operator->") {
        struct S {
            int a;
        };

        Result<S> result(S{1});
        CHECK(result->a == 1);
        CHECK(std::as_const(result)->a == 1);
    }

    SECTION("operator*") {
        Result<int> result(1);
        CHECK(*result == 1);
        CHECK(*std::as_const(result) == 1);

        SECTION("rvalue") {
            CHECK(*Result<int>(1) == 1);
        }
    }

    SECTION("code") {
        CHECK(Result<int>(1).code() == UA_STATUSCODE_GOOD);
        CHECK(Result<int>(badResult).code() == badCode);
    }

    SECTION("operator bool") {
        CHECK(static_cast<bool>(Result<int>(1)));
        CHECK_FALSE(static_cast<bool>(Result<int>(badResult)));
    }

    SECTION("hasValue") {
        CHECK(Result<int>().hasValue());
        CHECK_FALSE(Result<int>(badResult).hasValue());
    }

    SECTION("value") {
        Result<int> resultDefault;
        CHECK(resultDefault.value() == 0);

        Result<int> result(1);
        CHECK(result.value() == 1);
        CHECK(std::as_const(result).value() == 1);

        Result<int> resultError(badResult);
        CHECK_THROWS_AS(resultError.value(), BadStatus);
        CHECK_THROWS_AS(std::as_const(resultError).value(), BadStatus);

        SECTION("rvalue") {
            CHECK(Result<int>(1).value() == 1);
            CHECK_THROWS_AS(Result<int>(badResult).value(), BadStatus);
        }
    }

    SECTION("valueOr") {
        Result<int> result(1);
        CHECK(result.valueOr(2) == 1);

        Result<int> resultError(badResult);
        CHECK(resultError.valueOr(2) == 2);

        SECTION("rvalue") {
            CHECK(Result<int>(1).valueOr(2) == 1);
            CHECK(Result<int>(badResult).valueOr(2) == 2);
        }
    }

    SECTION("transform") {
        auto func = [](int value) { return 2 * value; };
        CHECK(Result<int>(1).transform(func).value() == 2);
        CHECK(Result<int>(1).transform(func).code().isGood());

        CHECK_THROWS_AS(Result<int>(badResult).transform(func).value(), BadStatus);
        CHECK(Result<int>(badResult).transform(func).code().isBad());

        SECTION("void return") {
            CHECK(Result<int>(1).transform([](auto&&) {}).code().isGood());
            CHECK(Result<int>(badResult).transform([](auto&&) {}).code().isBad());
        }
    }

    SECTION("andThen") {
        auto func = [](int value) -> Result<double> {
            if (value > 0) {
                return 2.2 * value;
            }
            return badResult;
        };

        CHECK(Result<int>(1).andThen(func).value() == 2.2);
        CHECK(Result<int>(1).andThen(func).code().isGood());

        CHECK_THROWS_AS(Result<int>(badResult).andThen(func).value(), BadStatus);
        CHECK(Result<int>(badResult).andThen(func).code().isBad());

        CHECK_THROWS_AS(Result<int>(0).andThen(func).value(), BadStatus);
        CHECK(Result<int>(0).andThen(func).code().isBad());
    }

    SECTION("andThen with StatusCode") {
        auto func = [](int value, StatusCode) -> Result<double> {
            if (value > 0) {
                return 2.2 * value;
            }
            return badResult;
        };

        CHECK(Result<int>(1).andThen(func).value() == 2.2);
        CHECK(Result<int>(1).andThen(func).code().isGood());

        CHECK_THROWS_AS(Result<int>(badResult).andThen(func).value(), BadStatus);
        CHECK(Result<int>(badResult).andThen(func).code().isBad());

        CHECK_THROWS_AS(Result<int>(0).andThen(func).value(), BadStatus);
        CHECK(Result<int>(0).andThen(func).code().isBad());
    }

    SECTION("orElse") {
        auto func = [](StatusCode) -> Result<int> { return 0; };

        CHECK(Result<int>(1).orElse(func).value() == 1);
        CHECK(Result<int>(1).orElse(func).code().isGood());

        CHECK(Result<int>(badResult).orElse(func).value() == 0);
        CHECK(Result<int>(badResult).orElse(func).code().isGood());
    }
}

TEST_CASE("Result (void template specialization)") {
    SECTION("code") {
        CHECK(Result<void>().code() == UA_STATUSCODE_GOOD);
        CHECK(Result<void>(badResult).code() == badCode);
    }

    SECTION("operator bool") {
        CHECK(static_cast<bool>(Result<void>()));
        CHECK_FALSE(static_cast<bool>(Result<void>(badResult)));
    }

    SECTION("hasValue") {
        CHECK(Result<void>().hasValue());
        CHECK_FALSE(Result<void>(badResult).hasValue());
    }

    SECTION("value") {
        CHECK_NOTHROW(Result<void>().value());
        CHECK_THROWS_AS(Result<void>(badResult).value(), BadStatus);
    }
}

TEST_CASE("tryInvoke") {
    SECTION("int return") {
        auto result = detail::tryInvoke([] { return 1; });
        CHECK(result.code() == UA_STATUSCODE_GOOD);
        CHECK(result.value() == 1);
    }

    SECTION("void return") {
        auto result = detail::tryInvoke([] { return; });
        CHECK(result.code() == UA_STATUSCODE_GOOD);
    }

    SECTION("StatusCode return") {
        auto result = detail::tryInvoke([] { return badCode; });
        CHECK(result.code() == badCode);
    }

    SECTION("BadResult return") {
        auto result = detail::tryInvoke([] { return badResult; });
        CHECK(result.code() == badCode);
    }

    SECTION("BadStatus exception") {
        auto result = detail::tryInvoke([] { throw BadStatus(badCode); });
        CHECK(result.code() == badCode);
    }

    SECTION("Other exception types") {
        auto result = detail::tryInvoke([] { throw std::runtime_error("test"); });
        CHECK(result.code() == UA_STATUSCODE_BADINTERNALERROR);
    }
}
