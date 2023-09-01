#include "CustomLogger.h"

#include <cassert>
#include <cstdarg>  // va_list, va_copy
#include <cstdio>
#include <string>
#include <utility>  // move

namespace opcua {

static std::string printfFormatToString(const char* msg, va_list args) noexcept {
    va_list tmp{};  // NOLINT
    va_copy(tmp, args);  // NOLINT
    const int charsToWrite = std::vsnprintf(nullptr, 0, msg, tmp);  // NOLINT
    va_end(tmp);  // NOLINT
    std::string buffer(charsToWrite, ' ');
    const int charsWritten = std::vsnprintf(buffer.data(), buffer.size() + 1, msg, args);
    if (charsWritten < 0) {
        return {};
    }
    return buffer;
}

static void log(
    void* context, UA_LogLevel level, UA_LogCategory category, const char* msg, va_list args
) {
    assert(context != nullptr);
    const auto* instance = static_cast<CustomLogger*>(context);
    const Logger& logger = instance->getLogger();

    // skip if no logger set
    if (!logger) {
        return;
    }

    logger(
        static_cast<LogLevel>(level),
        static_cast<LogCategory>(category),
        printfFormatToString(msg, args)
    );
}

CustomLogger::CustomLogger(UA_Logger& logger)
    : nativeLogger_(logger) {}

void CustomLogger::setLogger(Logger logger) {
    logger_ = std::move(logger);
    nativeLogger_.log = log;
    nativeLogger_.context = this;
    nativeLogger_.clear = nullptr;
}

const Logger& CustomLogger::getLogger() const noexcept {
    return logger_;
}

}  // namespace opcua
