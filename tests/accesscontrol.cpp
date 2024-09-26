#include <cstdint>

#include <doctest/doctest.h>

#include "open62541pp/config.hpp"
#include "open62541pp/plugins/accesscontrol.hpp"
#include "open62541pp/server.hpp"
#include "open62541pp/session.hpp"

using namespace opcua;

TEST_CASE("AccessControlDefault") {
    Server server;

    SUBCASE("getUserTokenPolicies") {
        AccessControlDefault ac(true, {{"username", "password"}});

        CHECK(ac.getUserTokenPolicies().size() == 2);
        CHECK(ac.getUserTokenPolicies()[0].getPolicyId() == "open62541-anonymous-policy");
        CHECK(ac.getUserTokenPolicies()[0].getTokenType() == UserTokenType::Anonymous);

        CHECK(ac.getUserTokenPolicies()[1].getPolicyId() == "open62541-username-policy");
        CHECK(ac.getUserTokenPolicies()[1].getTokenType() == UserTokenType::Username);
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
        CHECK(ac.getUserAccessLevel(session, {}) == static_cast<uint8_t>(0xFF));
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

class AccessControlTest : public AccessControlDefault {
public:
    using AccessControlDefault::AccessControlDefault;

    Bitmask<AccessLevel> getUserAccessLevel(
        [[maybe_unused]] Session& session, [[maybe_unused]] const NodeId& nodeId
    ) override {
        throw std::runtime_error("This exception should result in most restrictive access");
    }

    bool allowDeleteNode(
        [[maybe_unused]] Session& session, [[maybe_unused]] const DeleteNodesItem& item
    ) noexcept override {
        return false;
    }

    bool allowDeleteReference(
        [[maybe_unused]] Session& session, [[maybe_unused]] const DeleteReferencesItem& item
    ) noexcept override {
        return false;
    }
};

#if UAPP_OPEN62541_VER_GE(1, 3)
TEST_CASE("AccessControl plugin adapter") {
    Server server;
    AccessControlTest accessControl;
    UA_AccessControl native = accessControl.create();

    CHECK(native.context != nullptr);
    CHECK(native.userTokenPoliciesSize == 1);  // anonymous only
    CHECK(native.userTokenPolicies != nullptr);
    CHECK(native.userTokenPolicies[0].tokenType == UA_USERTOKENTYPE_ANONYMOUS);

    CHECK(native.activateSession != nullptr);
    CHECK(
        native.activateSession(
            server.handle(),
            &native,
            nullptr,  // endpoint description
            nullptr,  // secure channel remote certificate
            nullptr,  // session id
            nullptr,  // user identity token
            nullptr  // session context
        ) == UA_STATUSCODE_GOOD
    );

    CHECK(native.closeSession != nullptr);
    CHECK_NOTHROW(native.closeSession(
        server.handle(),
        &native,
        nullptr,  // session id
        nullptr  // session context
    ));

    CHECK(native.getUserRightsMask != nullptr);
    CHECK(
        native.getUserRightsMask(
            server.handle(),
            &native,
            nullptr,  // session id
            nullptr,  // session context
            nullptr,  // node id
            nullptr  // node context
        ) == 0xFFFFFFFF
    );

    CHECK(native.getUserAccessLevel != nullptr);
    CHECK(
        native.getUserAccessLevel(
            server.handle(),
            &native,
            nullptr,  // session id
            nullptr,  // session context
            nullptr,  // node id
            nullptr  // node context
        ) == 0x00  // most restrictive access due to exception
    );

    CHECK(native.getUserExecutable != nullptr);
    CHECK(
        native.getUserExecutable(
            server.handle(),
            &native,
            nullptr,  // session id
            nullptr,  // session context
            nullptr,  // method id
            nullptr  // method context
        ) == true
    );

    CHECK(native.getUserExecutableOnObject != nullptr);
    CHECK(
        native.getUserExecutableOnObject(
            server.handle(),
            &native,
            nullptr,  // session id
            nullptr,  // session context
            nullptr,  // method id
            nullptr,  // method context
            nullptr,  // object id
            nullptr  // object context
        ) == true
    );

    CHECK(native.allowAddNode != nullptr);
    CHECK(
        native.allowAddNode(
            server.handle(),
            &native,
            nullptr,  // session id
            nullptr,  // session context
            nullptr  // item
        ) == true
    );

    CHECK(native.allowAddReference != nullptr);
    CHECK(
        native.allowAddReference(
            server.handle(),
            &native,
            nullptr,  // session id
            nullptr,  // session context
            nullptr  // item
        ) == true
    );

    CHECK(native.allowDeleteNode != nullptr);
    CHECK(
        native.allowDeleteNode(
            server.handle(),
            &native,
            nullptr,  // session id
            nullptr,  // session context
            nullptr  // item
        ) == false
    );

    CHECK(native.allowDeleteReference != nullptr);
    CHECK(
        native.allowDeleteReference(
            server.handle(),
            &native,
            nullptr,  // session id
            nullptr,  // session context
            nullptr  // item
        ) == false
    );

    CHECK(native.allowBrowseNode != nullptr);
    CHECK(
        native.allowBrowseNode(
            server.handle(),
            &native,
            nullptr,  // session id
            nullptr,  // session context
            nullptr,  // node id
            nullptr  // node context
        ) == true
    );

#ifdef UA_ENABLE_SUBSCRIPTIONS
    CHECK(native.allowTransferSubscription != nullptr);
    CHECK(
        native.allowTransferSubscription(
            server.handle(),
            &native,
            nullptr,  // old session id
            nullptr,  // old session context
            nullptr,  // new session id
            nullptr  // new session context
        ) == true
    );
#endif

#ifdef UA_ENABLE_HISTORIZING
    CHECK(native.allowHistoryUpdateUpdateData != nullptr);
    CHECK(
        native.allowHistoryUpdateUpdateData(
            server.handle(),
            &native,
            nullptr,  // session id
            nullptr,  // session context
            nullptr,  // node id
            UA_PERFORMUPDATETYPE_INSERT,
            nullptr  // value
        ) == true
    );

    CHECK(native.allowHistoryUpdateDeleteRawModified != nullptr);
    CHECK(
        native.allowHistoryUpdateDeleteRawModified(
            server.handle(),
            &native,
            nullptr,  // session id
            nullptr,  // session context
            nullptr,  // node id
            {},  // start timestamp,
            {},  // stop timestamp,
            true
        ) == true
    );
#endif
}
#endif
