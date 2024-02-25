#pragma once

#include "CustomDataTypes.h"
#include "CustomLogger.h"
#include "open62541_impl.h"  // UA_ServerConfig

namespace opcua {

class ServerConfig {
public:
    ServerConfig(UA_ServerConfig& config)
        : config_(config) {}

    void setLogger(Logger logger) {
        customLogger_.set(config_.logger, std::move(logger));
    }

    void setCustomDataTypes(std::vector<DataType> dataTypes) {
        customDataTypes_.set(config_.customDataTypes, std::move(dataTypes));
    }

    constexpr UA_ServerConfig* operator->() noexcept {
        return &config_;
    }

    constexpr const UA_ServerConfig* operator->() const noexcept {
        return &config_;
    }

    constexpr UA_ServerConfig* handle() noexcept {
        return &config_;
    }

    constexpr const UA_ServerConfig* handle() const noexcept {
        return &config_;
    }

private:
    UA_ServerConfig& config_;
    CustomDataTypes customDataTypes_;
    CustomLogger customLogger_;
};

}  // namespace opcua
