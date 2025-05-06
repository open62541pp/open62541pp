#include "open62541pp/plugin/log.hpp"

#include <cassert>
#include <cstdarg>  // va_list
#include <cstdio>  // vsnprintf
#include <string>

#include "open62541pp/config.hpp"

namespace opcua {

static void toString(std::string& buffer, const char* format, va_list args) {
    buffer.clear();
    // NOLINTBEGIN
    va_list argsCopy{};
    va_copy(argsCopy, args);
    const int charsToWrite = std::vsnprintf(nullptr, 0, format, argsCopy);
    va_end(argsCopy);
    // NOLINTEND
    if (charsToWrite < 0) {
        return;
    }
    buffer.resize(charsToWrite);
    const int charsWritten = std::vsnprintf(buffer.data(), buffer.size() + 1, format, args);
    if (charsWritten < 0 || charsWritten > charsToWrite) {
        buffer.clear();
    }
}

static void logNative(
    void* context, UA_LogLevel level, UA_LogCategory category, const char* msg, va_list args
) {
    assert(context != nullptr);
    static thread_local std::string buffer(256, '\0');
    toString(buffer, msg, args);
    static_cast<LoggerBase*>(context)->log(
        static_cast<LogLevel>(level), static_cast<LogCategory>(category), buffer
    );
}

UA_Logger LoggerBase::create(bool ownsAdapter) {
    UA_Logger native{};
    native.log = logNative;
    native.context = this;
#if UAPP_OPEN62541_VER_GE(1, 4)
    if (ownsAdapter) {
        native.clear = [](UA_Logger* logger) {
            if (logger != nullptr) {
                delete static_cast<LoggerBase*>(logger->context);  // NOLINT
                logger->context = nullptr;
                UA_free(logger);
                logger = nullptr;
            }
        };
    } else {
        native.clear = [](UA_Logger* logger) {
            UA_free(logger);
            logger = nullptr;
        };
    }
#else
    if (ownsAdapter) {
        native.clear = [](void* context) {
            delete static_cast<LoggerBase*>(context);  // NOLINT
            context = nullptr;
        };
    }
#endif
    return native;
}

namespace detail {
void clear(UA_Logger& logger) noexcept {
    if (logger.clear != nullptr) {
#if UAPP_OPEN62541_VER_GE(1, 4)
        // Open62541 v1.4 transitioned to pointers for UA_Logger instances.
        // The clear function doesn't clear the context anymore but frees the memory and
        // consequently invalidates pointers like UA_EventLoop.logger.
        // Workaround to free dynamic context but not the existing logger instance:
        // 1. allocate new logger instance
        // 2. shallow copy the existing logger
        // 3. clear & free logger copy
        auto* loggerCopy = static_cast<UA_Logger*>(UA_malloc(sizeof(UA_Logger)));
        *loggerCopy = logger;  // shallow copy
        logger.clear(loggerCopy);
#else
        logger.clear(logger.context);
#endif
        logger = {};
    }
}
}  // namespace detail

}  // namespace opcua
