#pragma once

#include <vector>

#include "open62541pp/plugin/accesscontrol.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/types.hpp"

namespace opcua {

/// Login credentials.
struct Login {
    String username;
    String password;
};

/**
 * Default access control.
 *
 * This class implements the same logic as @ref UA_AccessControl_default().
 * The log-in can be anonymous or username-password. A logged-in user has all access rights.
 *
 * @warning Use less permissive access control in production!
 */
class AccessControlDefault : public AccessControlBase {
public:
    explicit AccessControlDefault(bool allowAnonymous = true, Span<const Login> logins = {});

    Span<UserTokenPolicy> getUserTokenPolicies() override;

    StatusCode activateSession(
        Session& session,
        const EndpointDescription& endpointDescription,
        const ByteString& secureChannelRemoteCertificate,
        const ExtensionObject& userIdentityToken,
        void*& sessionContext
    ) override;

    void closeSession(Session& session) override;

    Bitmask<WriteMask> getUserRightsMask(Session& session, const NodeId& nodeId) override;

    Bitmask<AccessLevel> getUserAccessLevel(Session& session, const NodeId& nodeId) override;

    bool getUserExecutable(Session& session, const NodeId& methodId) override;

    bool getUserExecutableOnObject(Session& session, const NodeId& methodId, const NodeId& objectId)
        override;

    bool allowAddNode(Session& session, const AddNodesItem& item) override;

    bool allowAddReference(Session& session, const AddReferencesItem& item) override;

    bool allowDeleteNode(Session& session, const DeleteNodesItem& item) override;

    bool allowDeleteReference(Session& session, const DeleteReferencesItem& item) override;

    bool allowBrowseNode(Session& session, const NodeId& nodeId) override;

    bool allowTransferSubscription(Session& oldSession, Session& newSession) override;

    bool allowHistoryUpdate(
        Session& session,
        const NodeId& nodeId,
        PerformUpdateType performInsertReplace,
        const DataValue& value
    ) override;

    bool allowHistoryDelete(
        Session& session,
        const NodeId& nodeId,
        DateTime startTimestamp,
        DateTime endTimestamp,
        bool isDeleteModified
    ) override;

private:
    bool allowAnonymous_;
    std::vector<Login> logins_;
    std::vector<UserTokenPolicy> userTokenPolicies_;
};

}  // namespace opcua
