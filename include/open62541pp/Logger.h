#pragma once

#include <functional>
#include <string_view>

// forward declare
struct UA_Client;
struct UA_Server;

namespace opcua {

// forward declare
class Client;
class Server;

/**
 * Log level.
 * @see UA_LogLevel
 */
enum class LogLevel {
    Trace = 0,
    Debug,
    Info,
    Warning,
    Error,
    Fatal,
};

/**
 * Log category.
 * @see UA_LogCategory
 */
enum class LogCategory {
    Network = 0,
    SecureChannel,
    Session,
    Server,
    Client,
    Userland,
    SecurityPolicy,
};

/// Log function signature.
using Logger = std::function<void(LogLevel, LogCategory, std::string_view msg)>;

/// Generate log message with client's logger.
void log(UA_Client* client, LogLevel level, LogCategory category, std::string_view msg);

/// Generate log message with client's logger.
void log(Client& client, LogLevel level, LogCategory category, std::string_view msg);

/// Generate log message with servers's logger.
void log(UA_Server* server, LogLevel level, LogCategory category, std::string_view msg);

/// Generate log message with servers's logger.
void log(Server& server, LogLevel level, LogCategory category, std::string_view msg);

/* -------------------------------------- Utility functions ------------------------------------- */

/// Get name of log level.
constexpr std::string_view getLogLevelName(LogLevel level) {
    switch (level) {
    case LogLevel::Trace:
        return "trace";
    case LogLevel::Debug:
        return "debug";
    case LogLevel::Info:
        return "info";
    case LogLevel::Warning:
        return "warning";
    case LogLevel::Error:
        return "error";
    case LogLevel::Fatal:
        return "fatal";
    default:
        return "unknown";
    }
}

/// Get name of log category.
constexpr std::string_view getLogCategoryName(LogCategory category) {
    switch (category) {
    case LogCategory::Network:
        return "network";
    case LogCategory::SecureChannel:
        return "channel";
    case LogCategory::Session:
        return "session";
    case LogCategory::Server:
        return "server";
    case LogCategory::Client:
        return "client";
    case LogCategory::Userland:
        return "userland";
    case LogCategory::SecurityPolicy:
        return "securitypolicy";
    default:
        return "unknown";
    }
}

}  // namespace opcua
