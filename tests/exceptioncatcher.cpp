#include <exception>
#include <stdexcept>

#include <doctest/doctest.h>

#include "open62541pp/detail/exceptioncatcher.hpp"

TEST_CASE("ExceptionCatcher") {
    opcua::detail::ExceptionCatcher catcher;

    CHECK_FALSE(catcher.hasException());
    CHECK_NOTHROW(catcher.rethrow());

    SUBCASE("Set and rethrow exception") {
        catcher.setException(std::make_exception_ptr(std::runtime_error("Error")));
        CHECK(catcher.hasException());
        CHECK_THROWS_WITH_AS(catcher.rethrow(), "Error", std::runtime_error);

        CHECK_FALSE(catcher.hasException());
        CHECK_NOTHROW(catcher.rethrow());
    }

    SUBCASE("Invoke and catch exception") {
        catcher.invoke([] {});
        CHECK_FALSE(catcher.hasException());

        catcher.invoke([] { throw std::runtime_error("Error"); });
        CHECK(catcher.hasException());
        CHECK_THROWS_WITH_AS(catcher.rethrow(), "Error", std::runtime_error);
    }

    SUBCASE("Wrap callback") {
        auto wrapped = catcher.wrapCallback([](bool error) {
            if (error) {
                throw std::runtime_error("Error");
            }
        });

        wrapped(false);
        CHECK_FALSE(catcher.hasException());

        wrapped(true);
        CHECK(catcher.hasException());
        CHECK_THROWS_WITH_AS(catcher.rethrow(), "Error", std::runtime_error);
    }
}
