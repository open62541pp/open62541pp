#include <doctest/doctest.h>

#include "open62541pp/AccessControl.h"
#include "open62541pp/Server.h"

using namespace opcua;

TEST_CASE("AccessControlDefault") {
    Server server;

    SUBCASE("getUserTokenPolicies") {
        AccessControlDefault ac(server, true, {{"username", "password"}});
        std::string_view securityPolicyUriNone = "http://opcfoundation.org/UA/SecurityPolicy#None";

        const auto userTokenPolicies = ac.getUserTokenPolicies();
        CHECK(userTokenPolicies.size() == 2);
        CHECK(userTokenPolicies.at(0).getPolicyId() == "open62541-anonymous-policy");
        CHECK(userTokenPolicies.at(0).getTokenType() == UserTokenType::Anonymous);
        CHECK(userTokenPolicies.at(0).getSecurityPolicyUri() == securityPolicyUriNone);

        CHECK(userTokenPolicies.at(1).getPolicyId() == "open62541-username-policy");
        CHECK(userTokenPolicies.at(1).getTokenType() == UserTokenType::Username);
        CHECK(userTokenPolicies.at(1).getSecurityPolicyUri() == securityPolicyUriNone);
    }

    SUBCASE("activateSession") {
        for (bool allowAnonymous : {false, true}) {
            CAPTURE(allowAnonymous);

            AccessControlDefault ac(server, allowAnonymous, {{"username", "password"}});
            const EndpointDescription endpointDescription{};
            const ByteString secureChannelRemoteCertificate{};
            const NodeId sessionId{};
            AccessControlBase::SessionContext sessionContext{};

            const auto activateSessionWithToken = [&](const ExtensionObject& userIdentityToken) {
                return ac.activateSession(
                    endpointDescription,
                    secureChannelRemoteCertificate,
                    sessionId,
                    userIdentityToken,
                    sessionContext
                );
            };

            SUBCASE("Empty token") {
                const ExtensionObject userIdentityToken{};
                CHECK_EQ(
                    activateSessionWithToken(userIdentityToken),
                    allowAnonymous ? UA_STATUSCODE_GOOD : UA_STATUSCODE_BADIDENTITYTOKENINVALID
                );
            }

            SUBCASE("Unknown token") {
                IssuedIdentityToken token;
                CHECK_EQ(
                    activateSessionWithToken(ExtensionObject::fromDecoded(token)),
                    UA_STATUSCODE_BADIDENTITYTOKENINVALID
                );
            }

            SUBCASE("Anonymous login") {
                AnonymousIdentityToken token;
                token.getPolicyId() = String("open62541-anonymous-policy");
                CHECK_EQ(
                    activateSessionWithToken(ExtensionObject::fromDecoded(token)),
                    allowAnonymous ? UA_STATUSCODE_GOOD : UA_STATUSCODE_BADIDENTITYTOKENINVALID
                );
            }

            SUBCASE("Username and password") {
                UserNameIdentityToken token;
                CHECK_EQ(
                    activateSessionWithToken(ExtensionObject::fromDecoded(token)),
                    UA_STATUSCODE_BADIDENTITYTOKENINVALID
                );

                token.getPolicyId() = String("open62541-username-policy");
                CHECK_EQ(
                    activateSessionWithToken(ExtensionObject::fromDecoded(token)),
                    UA_STATUSCODE_BADIDENTITYTOKENINVALID
                );

                token.getUserName() = String("username");
                token.getPassword() = ByteString("wrongpassword");
                CHECK_EQ(
                    activateSessionWithToken(ExtensionObject::fromDecoded(token)),
                    UA_STATUSCODE_BADUSERACCESSDENIED
                );

                token.getPassword() = ByteString("password");
                CHECK_EQ(
                    activateSessionWithToken(ExtensionObject::fromDecoded(token)),
                    UA_STATUSCODE_GOOD
                );
            }
        }
    }

    SUBCASE("Access control callbacks (all permissive)") {
        AccessControlDefault ac(server);
        const NodeId sessionId{};
        AccessControlBase::SessionContext sessionContext{};

        CHECK(ac.getUserRightsMask(sessionId, sessionContext, {}, {}) == 0xFFFFFFFF);
        CHECK(ac.getUserAccessLevel(sessionId, sessionContext, {}, {}) == 0xFF);
        CHECK(ac.getUserExecutable(sessionId, sessionContext, {}, {}));
        CHECK(ac.getUserExecutableOnObject(sessionId, sessionContext, {}, {}, {}, {}));
        CHECK(ac.allowAddNode(sessionId, sessionContext, {}));
        CHECK(ac.allowAddReference(sessionId, sessionContext, {}));
        CHECK(ac.allowDeleteNode(sessionId, sessionContext, {}));
        CHECK(ac.allowDeleteReference(sessionId, sessionContext, {}));
        CHECK(ac.allowBrowseNode(sessionId, sessionContext, {}, {}));
#ifdef UA_ENABLE_SUBSCRIPTIONS
        CHECK(ac.allowTransferSubscription(sessionId, sessionContext, sessionId, sessionContext));
#endif
#ifdef UA_ENABLE_HISTORIZING
        CHECK(ac.allowHistoryUpdate(sessionId, sessionContext, {}, {}, {}));
        CHECK(ac.allowHistoryDelete(sessionId, sessionContext, {}, {}, {}, {}));
#endif
    }
}
