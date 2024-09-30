#pragma once

#include <functional>
#include <utility>  // move

#include "open62541pp/plugin/log.hpp"

namespace opcua {

/// Log function.
using Logger = std::function<void(LogLevel, LogCategory, std::string_view msg)>;

/**
 * Logger class that wraps a `Logger`.
 */
class LoggerDefault : public LoggerBase {
public:
    explicit LoggerDefault(Logger func)
        : func_(std::move(func)) {}

    void log(LogLevel level, LogCategory category, std::string_view msg) override {
        if (func_) {
            func_(level, category, msg);
        }
    }

private:
    Logger func_;
};

}  // namespace opcua
