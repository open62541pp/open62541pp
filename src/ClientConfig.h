#pragma once

#include "open62541pp/detail/open62541/client.h"  // UA_ClientConfig

#include "CustomDataTypes.h"
#include "CustomLogger.h"

namespace opcua {

class ClientConfig {
public:
    ClientConfig(UA_ClientConfig& config)
        : config_(config) {}

    void setLogger(Logger logger) {
        customLogger_.set(config_.logger, std::move(logger));
    }

    void setCustomDataTypes(std::vector<DataType> dataTypes) {
        customDataTypes_.set(config_.customDataTypes, std::move(dataTypes));
    }

    constexpr UA_ClientConfig* operator->() noexcept {
        return &config_;
    }

    constexpr const UA_ClientConfig* operator->() const noexcept {
        return &config_;
    }

    constexpr UA_ClientConfig* handle() noexcept {
        return &config_;
    }

    constexpr const UA_ClientConfig* handle() const noexcept {
        return &config_;
    }

private:
    UA_ClientConfig& config_;
    CustomLogger customLogger_;
    CustomDataTypes customDataTypes_;
};

}  // namespace opcua
