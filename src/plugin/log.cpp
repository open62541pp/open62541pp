#include "open62541pp/plugin/log.hpp"

#include <cstdarg>  // va_list
#include <string>

#include "open62541pp/config.hpp"
#include "open62541pp/client.hpp"
#include "open62541pp/config.hpp"
#include "open62541pp/detail/string_utils.hpp"  // detail::toString
#include "open62541pp/server.hpp"

namespace opcua {

static void logNative(
    void* context, UA_LogLevel level, UA_LogCategory category, const char* msg, va_list args
) {
    assert(context != nullptr);
    static_cast<LoggerBase*>(context)->log(
        static_cast<LogLevel>(level),
        static_cast<LogCategory>(category),
        detail::toString(msg, args)
    );
}

UA_Logger LoggerBase::create() {
    UA_Logger native{};
    native.log = logNative;
    native.context = this;
#if UAPP_OPEN62541_VER_GE(1, 4)
    native.clear = [](UA_Logger* ptr) { UA_free(ptr); };
#endif
    return native;
}

void LoggerBase::clear(UA_Logger& native) noexcept {
    if (native.clear != nullptr) {
#if UAPP_OPEN62541_VER_GE(1, 4)
        // TODO:
        // Open62541 v1.4 transitioned to pointers for UA_Logger instances.
        // The clear function doesn't clear the context anymore but frees the memory and
        // consequently invalidates pointers like UA_EventLoop.logger.
        // Neighter the open62541 loggers UA_Log_Syslog_log, UA_Log_Syslog_log, nor the
        // opcua::Logger needs to be cleared, so skip this for now.

        // native.clear(&native);
#else
        native.clear(native.context);
        native.context = nullptr;
#endif
    }
}

// void LoggerBase::clear(UA_Logger*& plugin) noexcept override {
//     if (plugin != nullptr) {
//         clear(*plugin);
//         plugin = nullptr;
//     }
// }

/* ---------------------------------------------------------------------------------------------- */

void log(const UA_Logger* logger, LogLevel level, LogCategory category, std::string_view msg) {
    if (logger == nullptr || logger->log == nullptr) {
        return;
    }
    va_list args{};  // NOLINT
    logger->log(
        logger->context,
        static_cast<UA_LogLevel>(level),
        static_cast<UA_LogCategory>(category),
        std::string(msg).c_str(),
        args  // NOLINT
    );
}

void log(const UA_Logger& logger, LogLevel level, LogCategory category, std::string_view msg) {
    log(&logger, level, category, msg);
}

template <typename T>
static UA_Logger* getLogger(T* connection) noexcept {
    auto* config = detail::getConfig(connection);
    if (config == nullptr) {
        return nullptr;
    }
#if UAPP_OPEN62541_VER_GE(1, 4)
    return config->logging;
#else
    return &config->logger;
#endif
}

void log(UA_Client* client, LogLevel level, LogCategory category, std::string_view msg) {
    log(getLogger(client), level, category, msg);
}

void log(Client& client, LogLevel level, LogCategory category, std::string_view msg) {
    log(client.handle(), level, category, msg);
}

void log(UA_Server* server, LogLevel level, LogCategory category, std::string_view msg) {
    log(getLogger(server), level, category, msg);
}

void log(Server& server, LogLevel level, LogCategory category, std::string_view msg) {
    log(server.handle(), level, category, msg);
}

}  // namespace opcua
