#include <string>
#include <utility>

#include <doctest/doctest.h>

#include "open62541pp/client.hpp"
#include "open62541pp/plugin/log.hpp"
#include "open62541pp/server.hpp"

#include "client_config.hpp"
#include "server_config.hpp"

using namespace opcua;

TEST_CASE_TEMPLATE("Config", T, ClientConfig, ServerConfig) {
    SUBCASE("Default constructor") {
        T config;
    }
    SUBCASE("Move constructor") {
        T other;
        T config(std::move(other));
    }
    SUBCASE("Move assignment") {
        T other;
        T config = std::move(other);
    }

    T config;
}

TEST_CASE_TEMPLATE("Connection", T, Client, Server) {
    SUBCASE("Default constructor") {
        T connection;
    }
    SUBCASE("Move constructor") {
        T other;
        T connection(std::move(other));
    }
    SUBCASE("Move assignment") {
        T other;
        T connection = std::move(other);
    }

    T connection;

    SUBCASE("setLogger") {
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

        UA_LOG_INFO(detail::getLogger(connection), UA_LOGCATEGORY_USERLAND, "Message");
        CHECK(counter == 1);
        CHECK(lastLogLevel == LogLevel::Info);
        CHECK(lastLogCategory == LogCategory::Userland);
        CHECK(lastMessage == "Message");
    }
}
