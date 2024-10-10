#pragma once

#include <utility>  // forward, move

#include "open62541pp/detail/open62541/client.h"  // UA_ClientConfig
#include "open62541pp/plugin/log.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/wrapper.hpp"

#include "customdatatypes.hpp"

namespace opcua {

class ClientConfig {
public:
    ClientConfig(UA_ClientConfig& config)
        : config_(config) {}

    void setUserIdentityToken(ExtensionObject token) {
        asWrapper<ExtensionObject>(handle()->userIdentityToken) = std::move(token);
    }

    template <typename T>
    void setUserIdentityToken(T&& token) {
        setUserIdentityToken(ExtensionObject::fromDecodedCopy(std::forward<T>(token)));
    }

    void setLogger(LogFunction logger) {
        if (logger) {
            logger_ = std::make_unique<LoggerDefault>(std::move(logger));
#if UAPP_OPEN62541_VER_GE(1, 4)
            logger_->assign(handle()->logging);
#else
            logger_->assign(handle()->logger);
#endif
        }
    }

    void setCustomDataTypes(std::vector<DataType> dataTypes) {
        customDataTypes_.assign(std::move(dataTypes));
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
    CustomDataTypes customDataTypes_{config_.customDataTypes};
    std::unique_ptr<LoggerBase> logger_;
};

}  // namespace opcua
