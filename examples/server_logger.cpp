#include <iostream>
#include <string_view>

#include <open62541pp/server.hpp>

constexpr std::string_view toString(opcua::LogLevel level) {
    switch (level) {
    case opcua::LogLevel::Trace:
        return "trace";
    case opcua::LogLevel::Debug:
        return "debug";
    case opcua::LogLevel::Info:
        return "info";
    case opcua::LogLevel::Warning:
        return "warning";
    case opcua::LogLevel::Error:
        return "error";
    case opcua::LogLevel::Fatal:
        return "fatal";
    default:
        return "unknown";
    }
}

constexpr std::string_view toString(opcua::LogCategory category) {
    switch (category) {
    case opcua::LogCategory::Network:
        return "network";
    case opcua::LogCategory::SecureChannel:
        return "channel";
    case opcua::LogCategory::Session:
        return "session";
    case opcua::LogCategory::Server:
        return "server";
    case opcua::LogCategory::Client:
        return "client";
    case opcua::LogCategory::Userland:
        return "userland";
    case opcua::LogCategory::SecurityPolicy:
        return "securitypolicy";
    default:
        return "unknown";
    }
}

int main() {
    auto logger = [](auto level, auto category, auto msg) {
        std::cout << "[" << toString(level) << "] " << "[" << toString(category) << "] " << msg
                  << std::endl;
    };

    // Create server configuration and set logger
    opcua::ServerConfig config{4840, {}};
    config.setLogger(logger);

    // Construct server with config
    opcua::Server server{std::move(config)};

    // Alternatively, set logger after construction
    server.config().setLogger(logger);

    server.run();
}
