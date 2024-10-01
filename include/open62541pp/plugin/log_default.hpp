#pragma once

#include <functional>
#include <utility>  // move

#include "open62541pp/plugin/log.hpp"

namespace opcua {

/// Log function.
using LogFunction = std::function<void(LogLevel, LogCategory, std::string_view msg)>;

using Logger [[deprecated("use alias LogFunction instead")]] = LogFunction;

/**
 * Logger class that wraps a `LogFunction`.
 */
class LoggerDefault : public LoggerBase {
public:
    explicit LoggerDefault(LogFunction func)
        : func_(std::move(func)) {}

    void log(LogLevel level, LogCategory category, std::string_view msg) override {
        if (func_) {
            func_(level, category, msg);
        }
    }

private:
    LogFunction func_;
};

}  // namespace opcua
