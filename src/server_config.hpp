#pragma once

#include <memory>  // unique_ptr
#include <utility>  // move

#include "open62541pp/detail/open62541/server.h"  // UA_ServerConfig
#include "open62541pp/detail/types_handling.hpp"  // detail::deallocateArray
#include "open62541pp/exception.hpp"
#include "open62541pp/plugin/accesscontrol.hpp"
#include "open62541pp/plugin/log.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/types_composed.hpp"  // UserTokenPolicy
#include "open62541pp/wrapper.hpp"

namespace opcua {

class ServerConfig : public Wrapper<UA_ServerConfig> {
public:
    ServerConfig() {
        throwIfBad(UA_ServerConfig_setDefault(handle()));
    }

    explicit ServerConfig(UA_ServerConfig&& native)
        : Wrapper(std::exchange(native, {})) {}

    ~ServerConfig() {
        UA_ServerConfig_clean(handle());
    }

    ServerConfig(const ServerConfig&) = delete;

    ServerConfig(ServerConfig&& other) noexcept
        : Wrapper(std::exchange(other.native(), {})) {}

    ServerConfig& operator=(const ServerConfig&) = delete;

    ServerConfig& operator=(ServerConfig&& other) noexcept {
        if (this != &other) {
            native() = std::exchange(other.native(), {});
        }
        return *this;
    }

    void setLogger(LogFunction func) {
        if (func) {
            auto adapter = std::make_unique<LoggerDefault>(std::move(func));
            auto* logger = detail::getLogger(handle());
            assert(logger != nullptr);
            detail::clear(*logger);
            *logger = adapter.release()->create(true);
        }
    }

    void setAccessControl(AccessControlBase& accessControl) {
        detail::clear(native().accessControl);
        native().accessControl = accessControl.create(false);
        setHighestSecurityPolicyForUserTokenTransfer();
        copyUserTokenPoliciesToEndpoints();
    }

    void setAccessControl(std::unique_ptr<AccessControlBase>&& accessControl) {
        if (accessControl != nullptr) {
            detail::clear(native().accessControl);
            native().accessControl = accessControl->create(true);
            accessControl.release();
            setHighestSecurityPolicyForUserTokenTransfer();
            copyUserTokenPoliciesToEndpoints();
        }
    }

private:
    void copyUserTokenPoliciesToEndpoints() {
        // copy config->accessControl.userTokenPolicies -> config->endpoints[i].userIdentityTokens
        auto& ac = native().accessControl;
        for (size_t i = 0; i < native().endpointsSize; ++i) {
            auto& endpoint = native().endpoints[i];  // NOLINT
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
        auto& ac = native().accessControl;
        Span securityPolicies(native().securityPolicies, native().securityPoliciesSize);
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
};

}  // namespace opcua
