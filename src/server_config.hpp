#pragma once

#include <memory>  // unique_ptr
#include <utility>  // move
#include <vector>

#include "open62541pp/datatype.hpp"
#include "open62541pp/detail/open62541/server.h"  // UA_ServerConfig
#include "open62541pp/detail/types_handling.hpp"  // detail::deallocateArray
#include "open62541pp/plugin/accesscontrol.hpp"
#include "open62541pp/plugin/log.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/types_composed.hpp"  // UserTokenPolicy

namespace opcua {

class ServerConfig {
public:
    ServerConfig(UA_ServerConfig& config)
        : config_(config) {}

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

    void setAccessControl(AccessControlBase& accessControl) {
        detail::clear(handle()->accessControl);
        handle()->accessControl = accessControl.create(false);
        setHighestSecurityPolicyForUserTokenTransfer();
        copyUserTokenPoliciesToEndpoints();
    }

    void setAccessControl(std::unique_ptr<AccessControlBase>&& accessControl) {
        if (accessControl != nullptr) {
            detail::clear(handle()->accessControl);
            handle()->accessControl = accessControl->create(true);
            accessControl.release();
            setHighestSecurityPolicyForUserTokenTransfer();
            copyUserTokenPoliciesToEndpoints();
        }
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
    void copyUserTokenPoliciesToEndpoints() {
        // copy config->accessControl.userTokenPolicies -> config->endpoints[i].userIdentityTokens
        auto& ac = config_.accessControl;
        for (size_t i = 0; i < config_.endpointsSize; ++i) {
            auto& endpoint = config_.endpoints[i];  // NOLINT
            detail::deallocateArray(
                endpoint.userIdentityTokens,
                endpoint.userIdentityTokensSize,
                UA_TYPES[UA_TYPES_USERTOKENPOLICY]
            );
            endpoint.userIdentityTokens = detail::copyArray(
                ac.userTokenPolicies, ac.userTokenPoliciesSize, UA_TYPES[UA_TYPES_USERTOKENPOLICY]
            );
            endpoint.userIdentityTokensSize = ac.userTokenPoliciesSize;
        }
    }

    void setHighestSecurityPolicyForUserTokenTransfer() {
        auto& ac = config_.accessControl;
        Span securityPolicies(config_.securityPolicies, config_.securityPoliciesSize);
        Span userTokenPolicies(
            asWrapper<UserTokenPolicy>(ac.userTokenPolicies), ac.userTokenPoliciesSize
        );
        if (!securityPolicies.empty()) {
            const auto& highestSecurityPoliciyUri = securityPolicies.back().policyUri;
            for (auto& userTokenPolicy : userTokenPolicies) {
                if (userTokenPolicy.getTokenType() != UserTokenType::Anonymous &&
                    userTokenPolicy.getSecurityPolicyUri().empty()) {
                    userTokenPolicy.getSecurityPolicyUri() = String(highestSecurityPoliciyUri);
                }
            }
        }
    }

    UA_ServerConfig& config_;
    std::unique_ptr<std::vector<DataType>> types_;
    std::unique_ptr<UA_DataTypeArray> customDataTypes_;
};

}  // namespace opcua
