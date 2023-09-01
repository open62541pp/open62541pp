#include <doctest/doctest.h>

#include "open62541pp/AccessControl.h"
#include "open62541pp/Server.h"
#include "open62541pp/Session.h"
#include "open62541pp/types/DataValue.h"

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
            Session session(server, NodeId{});
            const EndpointDescription endpointDescription{};
            const ByteString secureChannelRemoteCertificate{};

            const auto activateSessionWithToken = [&](const ExtensionObject& userIdentityToken) {
                return ac.activateSession(
                    session, endpointDescription, secureChannelRemoteCertificate, userIdentityToken
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
        Session session(server, NodeId{});

        CHECK(ac.getUserRightsMask(session, {}) == 0xFFFFFFFF);
        CHECK(ac.getUserAccessLevel(session, {}) == 0xFF);
        CHECK(ac.getUserExecutable(session, {}));
        CHECK(ac.getUserExecutableOnObject(session, {}, {}));
        CHECK(ac.allowAddNode(session, {}));
        CHECK(ac.allowAddReference(session, {}));
        CHECK(ac.allowDeleteNode(session, {}));
        CHECK(ac.allowDeleteReference(session, {}));
        CHECK(ac.allowBrowseNode(session, {}));
        CHECK(ac.allowTransferSubscription(session, session));
        CHECK(ac.allowHistoryUpdate(session, {}, {}, {}));
        CHECK(ac.allowHistoryDelete(session, {}, {}, {}, {}));
    }
}
