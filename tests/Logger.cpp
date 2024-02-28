#include <string>

#include <doctest/doctest.h>

#include "open62541pp/Client.h"
#include "open62541pp/Logger.h"
#include "open62541pp/Server.h"

using namespace opcua;

TEST_CASE_TEMPLATE("Log with custom logger", T, Server, Client) {
    T serverOrClient;

    static size_t counter = 0;
    static LogLevel lastLogLevel{};
    static LogCategory lastLogCategory{};
    static std::string lastMessage{};

    serverOrClient.setLogger([&](LogLevel level, LogCategory category, std::string_view message) {
        counter++;
        lastLogLevel = level;
        lastLogCategory = category;
        lastMessage = message;
    });

    // passing a nullptr should do nothing
    serverOrClient.setLogger(nullptr);

    SUBCASE("Wrapper") {
        counter = 0;
        log(serverOrClient, LogLevel::Info, LogCategory::Server, "Message");
        CHECK(counter == 1);
        CHECK(lastLogLevel == LogLevel::Info);
        CHECK(lastLogCategory == LogCategory::Server);
        CHECK(lastMessage == "Message");
    }

    SUBCASE("Native") {
        auto native = serverOrClient.handle();
        counter = 0;
        log(native, LogLevel::Warning, LogCategory::Server, "Message from native");
        CHECK(counter == 1);
        CHECK(lastLogLevel == LogLevel::Warning);
        CHECK(lastLogCategory == LogCategory::Server);
        CHECK(lastMessage == "Message from native");
    }

    SUBCASE("Native nullptr") {
        auto native = serverOrClient.handle();
        native = nullptr;
        counter = 0;
        log(native, LogLevel::Warning, LogCategory::Server, "Message from null");
        CHECK(counter == 0);
    }
}
