#include <string>

#include <doctest/doctest.h>

#include "open62541pp/client.hpp"
#include "open62541pp/plugin/log.hpp"
#include "open62541pp/server.hpp"

using namespace opcua;

TEST_CASE_TEMPLATE("Log with custom logger", T, Server, Client) {
    T connection;

    static size_t counter = 0;
    static LogLevel lastLogLevel{};
    static LogCategory lastLogCategory{};
    static std::string lastMessage{};

    connection.setLogger([&](LogLevel level, LogCategory category, std::string_view message) {
        counter++;
        lastLogLevel = level;
        lastLogCategory = category;
        lastMessage = message;
    });

    // passing a nullptr should do nothing
    connection.setLogger(nullptr);

    UA_LOG_INFO(detail::getLogger(connection), UA_LOGCATEGORY_SERVER, "Message");
    CHECK(counter == 1);
    CHECK(lastLogLevel == LogLevel::Info);
    CHECK(lastLogCategory == LogCategory::Server);
    CHECK(lastMessage == "Message");
}
