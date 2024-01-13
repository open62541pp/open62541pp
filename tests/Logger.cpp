#include <string>

#include <doctest/doctest.h>

#include "open62541pp/Client.h"
#include "open62541pp/Logger.h"
#include "open62541pp/Server.h"

using namespace opcua;

TEST_CASE_TEMPLATE("Log with custom logger", T, Server, Client) {
    T serverOrClient;

    static LogLevel lastLogLevel{};
    static LogCategory lastLogCategory{};
    static std::string lastMessage{};

    serverOrClient.setLogger([&](LogLevel level, LogCategory category, std::string_view message) {
        lastLogLevel = level;
        lastLogCategory = category;
        lastMessage = message;
    });

    // passing a nullptr should do nothing
    serverOrClient.setLogger(nullptr);

    log(serverOrClient, LogLevel::Info, LogCategory::Server, "Message");
    CHECK(lastLogLevel == LogLevel::Info);
    CHECK(lastLogCategory == LogCategory::Server);
    CHECK(lastMessage == "Message");

    log(serverOrClient.handle(), LogLevel::Warning, LogCategory::Server, "Message from native");
    CHECK(lastLogLevel == LogLevel::Warning);
    CHECK(lastLogCategory == LogCategory::Server);
    CHECK(lastMessage == "Message from native");
}
