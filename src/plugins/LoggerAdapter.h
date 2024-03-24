#pragma once

#include <cassert>

#include "open62541pp/Logger.h"  // Logger
#include "open62541pp/detail/open62541/common.h"  // UA_Logger
#include "open62541pp/detail/string_utils.h"  // detail::toString
#include "open62541pp/plugins/PluginAdapter.h"

namespace opcua {

class LoggerAdapter final : public PluginAdapter<UA_Logger> {
public:
    explicit LoggerAdapter(Logger logger)
        : logger_(std::move(logger)) {}

    void clear(UA_Logger& native) noexcept override {
        if (native.clear != nullptr) {
            native.clear(native.context);
            native.context = nullptr;
        }
    }

    UA_Logger create() override {
        return {log, &logger_, nullptr};
    }

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
