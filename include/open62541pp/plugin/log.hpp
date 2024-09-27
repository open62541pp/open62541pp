#pragma once

#include <functional>
#include <string_view>
#include <utility>  // move

#include "open62541pp/detail/open62541/common.h"  // UA_LogLevel, UA_LogCategory, UA_Logger
#include "open62541pp/plugin/pluginadapter.hpp"

namespace opcua {

/**
 * Log level.
 * @see UA_LogLevel
 */
enum class LogLevel {
    Trace = UA_LOGLEVEL_TRACE,
    Debug = UA_LOGLEVEL_DEBUG,
    Info = UA_LOGLEVEL_INFO,
    Warning = UA_LOGLEVEL_WARNING,
    Error = UA_LOGLEVEL_ERROR,
    Fatal = UA_LOGLEVEL_FATAL,
};

/**
 * Log category.
 * @see UA_LogCategory
 */
enum class LogCategory {
    Network = UA_LOGCATEGORY_NETWORK,
    SecureChannel = UA_LOGCATEGORY_SECURECHANNEL,
    Session = UA_LOGCATEGORY_SESSION,
    Server = UA_LOGCATEGORY_SERVER,
    Client = UA_LOGCATEGORY_CLIENT,
    Userland = UA_LOGCATEGORY_USERLAND,
    SecurityPolicy = UA_LOGCATEGORY_SECURITYPOLICY,
};

/**
 * Logger base class.
 * 
 * Custom logger can be implemented by deriving from this class and overwriting the log function.
 */
class LoggerBase : public PluginAdapter<UA_Logger> {
public:
    virtual void log(LogLevel level, LogCategory category, std::string_view msg) = 0;

    UA_Logger create() override;
    void clear(UA_Logger& native) noexcept override;
};

/// Log function signature.
using Logger = std::function<void(LogLevel, LogCategory, std::string_view msg)>;

/**
 * Logger class that wraps a log handler.
 */
class LoggerHandler : public LoggerBase {
public:
    LoggerHandler(Logger logger)
        : logger_(std::move(logger)) {}

    virtual void log(LogLevel level, LogCategory category, std::string_view msg) {
        if (logger_) {
            logger_(level, category, msg);
        }
    }

private:
    Logger logger_;
};

/* ---------------------------------------------------------------------------------------------- */

/// Log a message with a UA_Logger instance (pointer).
void log(const UA_Logger* logger, LogLevel level, LogCategory category, std::string_view msg);

/// Log a message with a UA_Logger instance (reference).
void log(const UA_Logger& logger, LogLevel level, LogCategory category, std::string_view msg);

}  // namespace opcua
