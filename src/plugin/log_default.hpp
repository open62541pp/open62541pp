#pragma once

#include <cassert>

#include "open62541pp/config.hpp"
#include "open62541pp/detail/open62541/common.h"  // UA_Logger
#include "open62541pp/detail/string_utils.hpp"  // detail::toString
#include "open62541pp/plugin/log.hpp"  // Logger
#include "open62541pp/plugin/pluginadapter.hpp"

namespace opcua {

class LoggerAdapter : public PluginAdapter<UA_Logger> {
public:
    explicit LoggerAdapter(Logger logger)
        : logger_(std::move(logger)) {}

    UA_Logger create() override {
        UA_Logger native{};
        native.log = log;
        native.context = &logger_;
#if UAPP_OPEN62541_VER_GE(1, 4)
        native.clear = [](UA_Logger* ptr) { UA_free(ptr); };
#endif
        return native;
    }

    void clear(UA_Logger& native) noexcept override {
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

    // void clear(UA_Logger*& plugin) noexcept override {
    //     if (plugin != nullptr) {
    //         clear(*plugin);
    //         plugin = nullptr;
    //     }
    // }

private:
    static void log(
        void* context, UA_LogLevel level, UA_LogCategory category, const char* msg, va_list args
    ) noexcept {
        assert(context != nullptr);
        auto& logger = *static_cast<Logger*>(context);
        if (logger) {
            logger(
                static_cast<LogLevel>(level),
                static_cast<LogCategory>(category),
                detail::toString(msg, args)
            );
        }
    }

    Logger logger_;
};

}  // namespace opcua
