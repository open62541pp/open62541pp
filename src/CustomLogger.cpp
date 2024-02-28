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
    const Logger& logger = instance->get();

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

void CustomLogger::set(UA_Logger& native, Logger logger) {
    if (!logger) {
        return;
    }

    if (native.clear != nullptr) {
        native.clear(native.context);
        native.context = nullptr;
    }
    logger_ = std::move(logger);
    native.log = log;
    native.context = this;
    native.clear = nullptr;
}

const Logger& CustomLogger::get() const noexcept {
    return logger_;
}

}  // namespace opcua
