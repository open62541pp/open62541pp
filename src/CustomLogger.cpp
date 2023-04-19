#include "CustomLogger.h"

#include <cassert>
#include <cstdarg>  // va_list, va_copy
#include <cstdio>
#include <utility>  // move
#include <vector>

namespace opcua {

static void log(
    void* context, UA_LogLevel level, UA_LogCategory category, const char* msg, va_list args
) {
    assert(context != nullptr);  // NOLINT
    const auto* instance = static_cast<CustomLogger*>(context);
    const Logger& logger = instance->getLogger();

    // skip if no logger set
    if (!logger) {
        return;
    }

    // convert printf format + args to string_view
    va_list tmp;  // NOLINT
    va_copy(tmp, args);  // NOLINT
    const int charsToWrite = std::vsnprintf(nullptr, 0, msg, tmp);  // NOLINT
    va_end(tmp);  // NOLINT
    std::vector<char> buffer(charsToWrite + 1);
    const int charsWritten = std::vsnprintf(buffer.data(), buffer.size(), msg, args);
    if (charsWritten < 0) {
        return;
    }
    const std::string_view sv(buffer.data(), buffer.size());

    logger(static_cast<LogLevel>(level), static_cast<LogCategory>(category), sv);
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
