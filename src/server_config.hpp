#pragma once

#include <memory>  // unique_ptr
#include <utility>  // move
#include <vector>

#include "open62541pp/datatype.hpp"
#include "open62541pp/detail/open62541/server.h"  // UA_ServerConfig
#include "open62541pp/detail/types_handling.hpp"  // detail::deallocateArray
#include "open62541pp/plugin/log.hpp"
#include "open62541pp/session.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/types_composed.hpp"  // UserTokenPolicy

#include "customdatatypes.hpp"
#include "plugin/pluginmanager.hpp"

namespace opcua {

class Server;
class AccessControlBase;

class ServerConfig {
public:
    ServerConfig(UA_ServerConfig& config)
        : config_(config) {}

    void setLogger(Logger logger) {
        if (logger) {
            logger_.assign(LoggerDefault(std::move(logger)));
        }
    }

    void setCustomDataTypes(std::vector<DataType> dataTypes) {
        customDataTypes_.assign(std::move(dataTypes));
    }

    void setAccessControl(AccessControlBase& accessControl) {
        accessControl_.assign(&accessControl);
        setHighestSecurityPolicyForUserTokenTransfer();
        copyUserTokenPoliciesToEndpoints();
    }

    void setAccessControl(std::unique_ptr<AccessControlBase> accessControl) {
        accessControl_.assign(std::move(accessControl));
        setHighestSecurityPolicyForUserTokenTransfer();
        copyUserTokenPoliciesToEndpoints();
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
    CustomDataTypes customDataTypes_{config_.customDataTypes};
#if UAPP_OPEN62541_VER_GE(1, 4)
    PluginManager<UA_Logger*> logger_{config_.logging};
#else
    PluginManager<UA_Logger> logger_{config_.logger};
#endif
    PluginManager<UA_AccessControl> accessControl_{config_.accessControl};
};

}  // namespace opcua
