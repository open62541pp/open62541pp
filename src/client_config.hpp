#pragma once

#include <memory>  // unique_ptr
#include <utility>  // forward, move
#include <vector>

#include "open62541pp/datatype.hpp"
#include "open62541pp/detail/open62541/client.h"  // UA_ClientConfig
#include "open62541pp/plugin/log.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/wrapper.hpp"

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
            auto adapter = std::make_unique<LoggerDefault>(std::move(logger));
#if UAPP_OPEN62541_VER_GE(1, 4)
            assert(handle()->logging != nullptr);
            auto& native = *handle()->logging;
#else
            auto& native = handle()->logger;
#endif
            detail::clear(native);
            native = adapter->create(true);
            adapter.release();
        }
    }

    void setCustomDataTypes(std::vector<DataType> types) {
        types_ = std::make_unique<std::vector<DataType>>(std::move(types));
        customDataTypes_ = std::make_unique<UA_DataTypeArray>(detail::createDataTypeArray(*types_));
        handle()->customDataTypes = customDataTypes_.get();
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
    std::unique_ptr<std::vector<DataType>> types_;
    std::unique_ptr<UA_DataTypeArray> customDataTypes_;
};

}  // namespace opcua
