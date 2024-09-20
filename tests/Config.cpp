#include <doctest/doctest.h>

#include "open62541pp/AccessControl.h"

#include "ServerConfig.h"

using namespace opcua;

TEST_CASE("ServerConfig") {
    UA_ServerConfig native{};
    UA_ServerConfig_setDefault(&native);

    ServerConfig config(native);

    SUBCASE("AccessControl") {
        SUBCASE("Copy user token policies to endpoints") {
            CHECK(config->endpointsSize > 0);

            // delete endpoint user identity tokens first
            for (size_t i = 0; i < config->endpointsSize; ++i) {
                auto& endpoint = config->endpoints[i];
                UA_Array_delete(
                    endpoint.userIdentityTokens,
                    endpoint.userIdentityTokensSize,
                    &UA_TYPES[UA_TYPES_USERTOKENPOLICY]
                );
                endpoint.userIdentityTokens = (UA_UserTokenPolicy*)UA_EMPTY_ARRAY_SENTINEL;
                endpoint.userIdentityTokensSize = 0;
            }

            AccessControlDefault accessControl;
            config.setAccessControl(accessControl);
            auto& ac = config->accessControl;

            for (size_t i = 0; i < config->endpointsSize; ++i) {
                auto& endpoint = config->endpoints[i];
                CHECK(endpoint.userIdentityTokensSize == ac.userTokenPoliciesSize);
            }
        }

        SUBCASE("Use highest security policy to transfer user tokens") {
            AccessControlDefault accessControl(true, {{"user", "password"}});
            config.setAccessControl(accessControl);
            auto& ac = config->accessControl;

            CHECK(ac.userTokenPoliciesSize == 2);

            CHECK(ac.userTokenPolicies[0].tokenType == UA_USERTOKENTYPE_ANONYMOUS);
            CHECK(asWrapper<String>(ac.userTokenPolicies[0].securityPolicyUri).empty());

            CHECK(ac.userTokenPolicies[1].tokenType == UA_USERTOKENTYPE_USERNAME);
            CHECK(
                asWrapper<String>(ac.userTokenPolicies[1].securityPolicyUri) ==
                "http://opcfoundation.org/UA/SecurityPolicy#None"
            );
        }
    }

    UA_ServerConfig_clean(&native);
}
