#include <utility>  // move

#include <doctest/doctest.h>

#include "open62541pp/detail/ScopeExit.h"

using namespace opcua;

TEST_CASE("ScopeExit") {
    SUBCASE("Common") {
        bool executed = false;
        {
            auto exit = detail::ScopeExit([&] { executed = true; });
        }
        CHECK(executed);
    }

    SUBCASE("Move constructor") {
        int executions = 0;
        {
            detail::ScopeExit exit1([&] { executions++; });
            detail::ScopeExit exit2(std::move(exit1));
        }
        CHECK(executions == 1);
    }

    SUBCASE("Release") {
        bool executed = false;
        {
            auto exit = detail::ScopeExit([&] { executed = true; });
            exit.release();
        }
        CHECK_FALSE(executed);
    }
}
