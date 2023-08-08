#include <doctest/doctest.h>

#include "open62541pp/AccessControl.h"
#include "open62541pp/Server.h"

#include "CustomAccessControl.h"
#include "open62541_impl.h"

using namespace opcua;

class AccessControlTest : public AccessControlDefault {
public:
    using AccessControlDefault::AccessControlDefault;

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

TEST_CASE("CustomAccessControl") {
    Server server;

    CustomAccessControl customAccessControl(server);
    AccessControlTest accessControl;
    UA_AccessControl& native = UA_Server_getConfig(server.handle())->accessControl;

    // reset to empty UA_AccessControl
    detail::clearUaAccessControl(native);

    CHECK(customAccessControl.getServer() == server);

    SUBCASE("Set non-existing access control") {
        CHECK_NOTHROW(customAccessControl.setAccessControl());
        CHECK(customAccessControl.getAccessControl() == nullptr);
        CHECK(native.context == nullptr);
    }

    SUBCASE("Set custom access control") {
        CHECK_NOTHROW(customAccessControl.setAccessControl(accessControl));
        CHECK(customAccessControl.getAccessControl() == &accessControl);
        CHECK(native.context != nullptr);

        CHECK(native.userTokenPoliciesSize == 1);  // anonymous only
        CHECK(native.userTokenPolicies != nullptr);
        CHECK(native.userTokenPolicies[0].tokenType == UA_USERTOKENTYPE_ANONYMOUS);  // NOLINT

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
            ) == 0xFF
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

#if UAPP_OPEN62541_VER_GE(1, 1)
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
#endif

#if UAPP_OPEN62541_VER_GE(1, 2) && defined(UA_ENABLE_SUBSCRIPTIONS)
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

    SUBCASE("Reset custom access control") {
        CHECK_NOTHROW(customAccessControl.setAccessControl(accessControl));
        CHECK(native.context != nullptr);
        auto* oldContext = native.context;

        // manipulate native object
        native.context = nullptr;

        // reset should re-populate native object
        CHECK_NOTHROW(customAccessControl.setAccessControl());
        CHECK(native.context != nullptr);
        CHECK(native.context == oldContext);
    }

    SUBCASE("Store active sessions") {
        CHECK_NOTHROW(customAccessControl.setAccessControl(accessControl));
        CHECK(customAccessControl.getSessions().empty());

        // activate session
        NodeId sessionId(0, 1000);
        native.activateSession(
            server.handle(),
            &native,
            nullptr,  // endpoint description
            nullptr,  // secure channel remote certificate
            sessionId.handle(),  // session id
            nullptr,  // user identity token
            nullptr  // session context
        );
        CHECK(customAccessControl.getSessions().size() == 1);
        CHECK(customAccessControl.getSessions().at(0).getConnection() == server);
        CHECK(customAccessControl.getSessions().at(0).getSessionId() == sessionId);

        // close session
        native.closeSession(
            server.handle(),
            &native,
            sessionId.handle(),  // session id
            nullptr  // session context
        );
        CHECK(customAccessControl.getSessions().empty());
    }
}
