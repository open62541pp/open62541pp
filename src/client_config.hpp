#pragma once

#include <utility>  // forward, move

#include "open62541pp/detail/open62541/client.h"  // UA_ClientConfig
#include "open62541pp/plugin/log.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/wrapper.hpp"

#include "customdatatypes.hpp"
#include "plugin/pluginmanager.hpp"

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
            logger_.assign(LoggerDefault(std::move(logger)));
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
#if UAPP_OPEN62541_VER_GE(1, 4)
    PluginManager<UA_Logger*> logger_{config_.logging};
#else
    PluginManager<UA_Logger> logger_{config_.logger};
#endif
};

}  // namespace opcua
