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

}  // namespace opcua
