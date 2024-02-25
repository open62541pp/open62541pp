#pragma once

#include <memory>  // unique_ptr
#include <utility>  // move
#include <vector>

#include "open62541pp/DataType.h"
#include "open62541pp/Session.h"
#include "open62541pp/Span.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/Composed.h"  // UserTokenPolicy
#include "open62541pp/detail/helper.h"

#include "CustomAccessControl.h"
#include "CustomDataTypes.h"
#include "CustomLogger.h"
#include "open62541_impl.h"  // UA_ServerConfig

namespace opcua {

class Server;
class AccessControlBase;

class ServerConfig {
public:
    ServerConfig(UA_ServerConfig& config, Server& server /* TODO: remove dependency */)
        : config_(config) {
        customAccessControl_.setServer(server);
    }

    void setLogger(Logger logger) {
        customLogger_.set(config_.logger, std::move(logger));
    }

    void setCustomDataTypes(std::vector<DataType> dataTypes) {
        customDataTypes_.set(config_.customDataTypes, std::move(dataTypes));
    }

    void setAccessControl(AccessControlBase& accessControl) {
        customAccessControl_.setAccessControl(config_.accessControl, accessControl);
        setHighestSecurityPolicyForUserTokenTransfer();
        copyUserTokenPoliciesToEndpoints();
    }

    void setAccessControl(std::unique_ptr<AccessControlBase> accessControl) {
        customAccessControl_.setAccessControl(config_.accessControl, std::move(accessControl));
        setHighestSecurityPolicyForUserTokenTransfer();
        copyUserTokenPoliciesToEndpoints();
    }

    // TODO: decouple from CustomAccessControl and ServerConfig
    std::vector<Session> getSessions() const {
        return customAccessControl_.getSessions();
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
    CustomLogger customLogger_;
    CustomDataTypes customDataTypes_;
    CustomAccessControl customAccessControl_;
};

}  // namespace opcua
