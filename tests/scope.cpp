#include <utility>  // move

#include <catch2/catch_test_macros.hpp>

#include "open62541pp/detail/scope.hpp"

using namespace opcua;

TEST_CASE("ScopeExit") {
    SECTION("Common") {
        bool executed = false;
        {
            auto exit = detail::ScopeExit([&] { executed = true; });
        }
        CHECK(executed);
    }

    SECTION("Move constructor") {
        int executions = 0;
        {
            detail::ScopeExit exit1([&] { executions++; });
            detail::ScopeExit exit2(std::move(exit1));
        }
        CHECK(executions == 1);
    }

    SECTION("Release") {
        bool executed = false;
        {
            auto exit = detail::ScopeExit([&] { executed = true; });
            exit.release();
        }
        CHECK_FALSE(executed);
    }
}
