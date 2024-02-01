#include <exception>
#include <stdexcept>

#include <doctest/doctest.h>

#include "open62541pp/detail/ExceptionHandler.h"

TEST_CASE("ExceptionHandler") {
    opcua::detail::ExceptionHandler handler;

    CHECK_FALSE(handler.hasException());
    CHECK_NOTHROW(handler.rethrow());

    SUBCASE("Set and rethrow exception") {
        handler.setException(std::make_exception_ptr(std::runtime_error("Error")));
        CHECK(handler.hasException());
        CHECK_THROWS_AS_MESSAGE(handler.rethrow(), std::runtime_error, "Error");

        CHECK_FALSE(handler.hasException());
        CHECK_NOTHROW(handler.rethrow());
    }

    SUBCASE("Invoke and catch exception") {
        handler.invoke([] {});
        CHECK_FALSE(handler.hasException());

        handler.invoke([] { throw std::runtime_error("Error"); });
        CHECK(handler.hasException());
        CHECK_THROWS_AS_MESSAGE(handler.rethrow(), std::runtime_error, "Error");
    }

    SUBCASE("Wrap callback") {
        auto wrapped = handler.wrapCallback([](bool error) {
            if (error) {
                throw std::runtime_error("Error");
            }
        });

        wrapped(false);
        CHECK_FALSE(handler.hasException());

        wrapped(true);
        CHECK(handler.hasException());
        CHECK_THROWS_AS_MESSAGE(handler.rethrow(), std::runtime_error, "Error");
    }
}
