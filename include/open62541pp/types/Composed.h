#pragma once

#include "open62541pp/TypeConverter.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/types/Builtin.h"

#include <string>
#include <vector>

// NOLINTNEXTLINE
#define UAPP_COMPOSED_WRAPPER_GETTER(WrapperType, member, getterName)                              \
    WrapperType& getterName() noexcept {                                                           \
        return asWrapper<WrapperType>(handle()->member);                                           \
    }                                                                                              \
    const WrapperType& getterName() const noexcept {                                               \
        return asWrapper<WrapperType>(handle()->member);                                           \
    }

namespace opcua {

/**
 * UA_ApplicationDescription wrapper class.
 * @ingroup TypeWrapper
 */
class ApplicationDescription
    : public TypeWrapper<UA_ApplicationDescription, UA_TYPES_APPLICATIONDESCRIPTION> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_WRAPPER_GETTER(String, applicationUri, getApplicationUri)
    UAPP_COMPOSED_WRAPPER_GETTER(String, productUri, getProductUri)
    UAPP_COMPOSED_WRAPPER_GETTER(LocalizedText, applicationName, getApplicationName)

    UA_ApplicationType getApplicationType() const noexcept {
        return handle()->applicationType;
    }

    UAPP_COMPOSED_WRAPPER_GETTER(String, gatewayServerUri, getGatewayServerUri)
    UAPP_COMPOSED_WRAPPER_GETTER(String, discoveryProfileUri, getDiscoveryProfileUri)

    std::vector<std::string> getDiscoveryUrls() const {
        return detail::fromNativeArray<std::string>(
            handle()->discoveryUrls, handle()->discoveryUrlsSize
        );
    }
};

/**
 * UA_UserTokenPolicy wrapper class.
 * @ingroup TypeWrapper
 */
class UserTokenPolicy : public TypeWrapper<UA_UserTokenPolicy, UA_TYPES_USERTOKENPOLICY> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_WRAPPER_GETTER(String, policyId, getPolicyId)

    UA_UserTokenType getTokenType() const noexcept {
        return handle()->tokenType;
    }

    UAPP_COMPOSED_WRAPPER_GETTER(String, issuedTokenType, getIssuedTokenType)
    UAPP_COMPOSED_WRAPPER_GETTER(String, issuerEndpointUrl, getIssuerEndpointUrl)
    UAPP_COMPOSED_WRAPPER_GETTER(String, securityPolicyUri, getSecurityPolicyUri)
};

/**
 * UA_EndpointDescription wrapper class.
 * @ingroup TypeWrapper
 */
class EndpointDescription
    : public TypeWrapper<UA_EndpointDescription, UA_TYPES_ENDPOINTDESCRIPTION> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_WRAPPER_GETTER(String, endpointUrl, getEndpointUrl)
    UAPP_COMPOSED_WRAPPER_GETTER(ApplicationDescription, server, getServer)
    UAPP_COMPOSED_WRAPPER_GETTER(ByteString, serverCertificate, getServerCertificate)

    UA_MessageSecurityMode getSecurityMode() const noexcept {
        return handle()->securityMode;
    }

    UAPP_COMPOSED_WRAPPER_GETTER(String, securityPolicyUri, getSecurityPolicyUri)

    std::vector<UserTokenPolicy> getUserIdentityTokens() const {
        return detail::fromNativeArray<UserTokenPolicy>(
            handle()->userIdentityTokens, handle()->userIdentityTokensSize
        );
    }

    UAPP_COMPOSED_WRAPPER_GETTER(String, transportProfileUri, getTransportProfileUri)

    UA_Byte getSecurityLevel() const noexcept {
        return handle()->securityLevel;
    }
};

}  // namespace opcua
