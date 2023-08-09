#include <iostream>
#include <string>

#include <doctest/doctest.h>

#include "open62541pp/Logger.h"
#include "open62541pp/Server.h"

using namespace opcua;

TEST_CASE("Log with custom logger") {
    Server server;

    LogLevel lastLogLevel{};
    LogCategory lastLogCategory{};
    std::string lastMessage{};

    server.setLogger([&](LogLevel level, LogCategory category, std::string_view message) {
        lastLogLevel = level;
        lastLogCategory = category;
        lastMessage = message;
    });

    log(server, LogLevel::Info, LogCategory::Server, "Some log message");
    CHECK(lastLogLevel == LogLevel::Info);
    CHECK(lastLogCategory == LogCategory::Server);
    CHECK(lastMessage == "Some log message");

    log(server.handle(), LogLevel::Warning, LogCategory::Server, "Some log message from native");
    CHECK(lastLogLevel == LogLevel::Warning);
    CHECK(lastLogCategory == LogCategory::Server);
    CHECK(lastMessage == "Some log message from native");
}
