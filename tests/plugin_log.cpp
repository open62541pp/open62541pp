#include <string>

#include <doctest/doctest.h>

#include "open62541pp/plugin/log.hpp"
#include "open62541pp/plugin/log_default.hpp"

using namespace opcua;

class LoggerTest : public LoggerBase {
public:
    void log(LogLevel level, LogCategory category, std::string_view msg) override {
        lastLevel = level;
        lastCategory = category;
        lastMessage = msg;
    }

    LogLevel lastLevel;
    LogCategory lastCategory;
    std::string lastMessage;
};

TEST_CASE("LoggerBase") {
    LoggerTest logger;
    auto native = logger.create();
    
    CHECK(native.context == &logger);
    CHECK(native.log != nullptr);

    va_list args{};
    native.log(native.context, UA_LOGLEVEL_INFO, UA_LOGCATEGORY_USERLAND, "message", args);

    CHECK(logger.lastLevel == LogLevel::Info);
    CHECK(logger.lastCategory == LogCategory::Userland);
    CHECK(logger.lastMessage == "message");

    logger.clear(native);
}

TEST_CASE("log") {
    struct LogContext {
        UA_LogLevel level;
        UA_LogCategory category;
        std::string message;
    };

    UA_Logger logger{};
    LogContext context;
    logger.context = &context;
    logger.log =
        [](void* logContext, UA_LogLevel level, UA_LogCategory category, const char* msg, va_list) {
            CHECK(logContext != nullptr);
            static_cast<LogContext*>(logContext)->level = level;
            static_cast<LogContext*>(logContext)->category = category;
            static_cast<LogContext*>(logContext)->message = msg;
        };

    log(logger, LogLevel::Info, LogCategory::Userland, "message");
    CHECK(context.level == UA_LOGLEVEL_INFO);
    CHECK(context.category == UA_LOGCATEGORY_USERLAND);
    CHECK(context.message == "message");
}
