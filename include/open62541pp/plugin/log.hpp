#pragma once

#include <string_view>

#include "open62541pp/config.hpp"
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
#if UAPP_OPEN62541_VER_GE(1, 4)
    Security = UA_LOGCATEGORY_SECURITY,
    EventLoop = UA_LOGCATEGORY_EVENTLOOP,
    PubSub = UA_LOGCATEGORY_PUBSUB,
    Discovery = UA_LOGCATEGORY_DISCOVERY,
#endif
};

/**
 * Logger base class.
 *
 * Custom logger can be implemented by deriving from this class and overwriting the log function.
 */
class LoggerBase : public PluginAdapter<UA_Logger> {
public:
    virtual void log(LogLevel level, LogCategory category, std::string_view msg) = 0;

    UA_Logger create(bool ownsAdapter) override;
};

namespace detail {
void clear(UA_Logger& logger) noexcept;
}  // namespace detail

}  // namespace opcua
