#include <doctest/doctest.h>

#include "open62541pp/AccessControl.h"
#include "open62541pp/Server.h"

using namespace opcua;

TEST_CASE("AccessControlDefault") {
    Server server;

    SUBCASE("getUserTokenPolicies") {
        AccessControlDefault ac(true, {{"username", "password"}});

        const auto userTokenPolicies = ac.getUserTokenPolicies();
        CHECK(userTokenPolicies.size() == 2);
        CHECK(userTokenPolicies.at(0).getPolicyId() == "open62541-anonymous-policy");
        CHECK(userTokenPolicies.at(0).getTokenType() == UserTokenType::Anonymous);

        CHECK(userTokenPolicies.at(1).getPolicyId() == "open62541-username-policy");
        CHECK(userTokenPolicies.at(1).getTokenType() == UserTokenType::Username);
    }

    SUBCASE("activateSession") {
        for (bool allowAnonymous : {false, true}) {
            CAPTURE(allowAnonymous);

            AccessControlDefault ac(allowAnonymous, {{"username", "password"}});
            const EndpointDescription endpointDescription{};
            const ByteString secureChannelRemoteCertificate{};
            const NodeId sessionId{};

            const auto activateSessionWithToken = [&](const ExtensionObject& userIdentityToken) {
                return ac.activateSession(
                    server,
                    sessionId,
                    endpointDescription,
                    secureChannelRemoteCertificate,
                    userIdentityToken
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
        AccessControlDefault ac;
        const NodeId sessionId{};

        CHECK(ac.getUserRightsMask(server, sessionId, {}) == 0xFFFFFFFF);
        CHECK(ac.getUserAccessLevel(server, sessionId, {}) == 0xFF);
        CHECK(ac.getUserExecutable(server, sessionId, {}));
        CHECK(ac.getUserExecutableOnObject(server, sessionId, {}, {}));
        CHECK(ac.allowAddNode(server, sessionId, {}));
        CHECK(ac.allowAddReference(server, sessionId, {}));
        CHECK(ac.allowDeleteNode(server, sessionId, {}));
        CHECK(ac.allowDeleteReference(server, sessionId, {}));
        CHECK(ac.allowBrowseNode(server, sessionId, {}));
#ifdef UA_ENABLE_SUBSCRIPTIONS
        CHECK(ac.allowTransferSubscription(server, sessionId, sessionId));
#endif
#ifdef UA_ENABLE_HISTORIZING
        CHECK(ac.allowHistoryUpdate(server, sessionId, {}, {}, {}));
        CHECK(ac.allowHistoryDelete(server, sessionId, {}, {}, {}, {}));
#endif
    }
}
