#include <chrono>
#include <thread>

#include <catch2/catch_test_macros.hpp>

#include "open62541pp/callback.hpp"
#include "open62541pp/server.hpp"

#include "helper/server_client_setup.hpp"

using namespace opcua;

TEST_CASE("Empty callback") {
    Server server;

    const auto id = addTimedCallback(server, nullptr, DateTime::now());
    CAPTURE(id);
    server.runIterate();
    CHECK_NOTHROW(removeCallback(server, id));
}

TEST_CASE("Timed callback") {
    Server server;

    bool executed = false;
    const auto id = addTimedCallback(server, [&] { executed = true; }, DateTime::now());
    CAPTURE(id);
    server.runIterate();
    CHECK(executed == true);

    CHECK_NOTHROW(removeCallback(server, id));
}

TEST_CASE("Repeated callback") {
    Server server;

    size_t counter = 0;
    const auto id = addRepeatedCallback(server, [&] { ++counter; }, 10);
    CAPTURE(id);
    server.runIterate();
    CHECK(counter == 0);

    SECTION("Initial interval") {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
        server.runIterate();
        CHECK(counter == 1);
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
        server.runIterate();
        CHECK(counter == 2);
    }

    SECTION("Changed interval") {
        CHECK_NOTHROW(changeRepeatedCallbackInterval(server, id, 1));
        std::this_thread::sleep_for(std::chrono::milliseconds{1});
        server.runIterate();
        CHECK(counter == 1);
        std::this_thread::sleep_for(std::chrono::milliseconds{1});
        server.runIterate();
        CHECK(counter == 2);
    }

    const auto counterBeforeRemove = counter;
    CHECK_NOTHROW(removeCallback(server, id));
    std::this_thread::sleep_for(std::chrono::milliseconds{10});
    server.runIterate();
    CHECK(counter == counterBeforeRemove);
}
