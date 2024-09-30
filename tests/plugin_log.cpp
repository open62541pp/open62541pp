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
