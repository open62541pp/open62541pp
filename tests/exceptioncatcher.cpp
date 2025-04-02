#include <exception>
#include <stdexcept>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "open62541pp/detail/exceptioncatcher.hpp"

using Catch::Matchers::Message;

TEST_CASE("ExceptionCatcher") {
    opcua::detail::ExceptionCatcher catcher;

    CHECK_FALSE(catcher.hasException());
    CHECK_NOTHROW(catcher.rethrow());

    SECTION("Set and rethrow exception") {
        catcher.setException(std::make_exception_ptr(std::runtime_error{"Error"}));
        CHECK(catcher.hasException());
        CHECK_THROWS_MATCHES(catcher.rethrow(), std::runtime_error, Message("Error"));

        CHECK_FALSE(catcher.hasException());
        CHECK_NOTHROW(catcher.rethrow());
    }

    SECTION("Invoke and catch exception") {
        catcher.invoke([] {});
        CHECK_FALSE(catcher.hasException());

        catcher.invoke([] { throw std::runtime_error{"Error"}; });
        CHECK(catcher.hasException());
        CHECK_THROWS_MATCHES(catcher.rethrow(), std::runtime_error, Message("Error"));
    }

    SECTION("Wrap callback") {
        auto wrapped = catcher.wrapCallback([](bool error) {
            if (error) {
                throw std::runtime_error{"Error"};
            }
        });

        wrapped(false);
        CHECK_FALSE(catcher.hasException());

        wrapped(true);
        CHECK(catcher.hasException());
        CHECK_THROWS_MATCHES(catcher.rethrow(), std::runtime_error, Message("Error"));
    }
}
