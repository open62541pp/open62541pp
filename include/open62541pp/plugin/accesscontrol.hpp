#pragma once

#include <string>
#include <vector>

#include "open62541pp/bitmask.hpp"
#include "open62541pp/common.hpp"  // AccessLevel, WriteMask
#include "open62541pp/detail/open62541/common.h"  // UA_AccessControl
#include "open62541pp/plugin/pluginadapter.hpp"
#include "open62541pp/session.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/types_composed.hpp"  // UserTokenPolicy, PerformUpdateType

namespace opcua {

/// Login credentials.
struct Login {
    std::string username;
    std::string password;
};

/**
 * Access control base class.
 *
 * Used to authenticate sessions and grant access rights accordingly.
 * Custom access control can be implemented by deriving from this class and overwriting the access
 * control callbacks.
 *
 * If exceptions are thrown within the access control callbacks, they are caught in the C callbacks
 * and will return the most restrictive access rights, e.g. `AccessLevel::None` in
 * `getUserAccessLevel` or `false` in `allowAddNode`. The exception will be logged (warning level).
 *
 * The `sessionId` can originally be both `NULL` in open62541.
 * This is the case when, for example, a MonitoredItem (the underlying Subscription) is detached
 * from its Session but continues to run.
 * This wrapper passes `session` by reference, so it can't be `NULL`.
 * Instead, a `session` with an empty `sessionId` will be passed.
 *
 * @see UA_AccessControl
 * @see https://www.open62541.org/doc/1.3/plugin_accesscontrol.html
 */
class AccessControlBase : public PluginAdapter<UA_AccessControl> {
public:
    /**
     * Get available user token policies.
     * If the `securityPolicyUri` is empty, the highest available security policy will be used to
     * transfer user tokens.
     * @note The returned span must be valid throughout the lifetime of the instance.
     */
    virtual Span<UserTokenPolicy> getUserTokenPolicies() = 0;

    /**
     * Authenticate a session.
     * The new session is rejected if a status code other than `UA_STATUSCODE_GOOD` is returned.
     */
    virtual StatusCode activateSession(
        Session& session,
        const EndpointDescription& endpointDescription,
        const ByteString& secureChannelRemoteCertificate,
        const ExtensionObject& userIdentityToken
    ) = 0;

    /// Deauthenticate a session and cleanup session context.
    virtual void closeSession(Session& session) = 0;

    /// Access control for all nodes.
    virtual Bitmask<WriteMask> getUserRightsMask(Session& session, const NodeId& nodeId) = 0;

    /// Additional access control for variable nodes.
    virtual Bitmask<AccessLevel> getUserAccessLevel(Session& session, const NodeId& nodeId) = 0;

    /// Additional access control for method nodes.
    virtual bool getUserExecutable(Session& session, const NodeId& methodId) = 0;

    /// Additional access control for calling a method node in the context of a specific object.
    virtual bool getUserExecutableOnObject(
        Session& session, const NodeId& methodId, const NodeId& objectId
    ) = 0;

    /// Allow adding a node.
    virtual bool allowAddNode(Session& session, const AddNodesItem& item) = 0;

    /// Allow adding a reference.
    virtual bool allowAddReference(Session& session, const AddReferencesItem& item) = 0;

    /// Allow deleting a node.
    virtual bool allowDeleteNode(Session& session, const DeleteNodesItem& item) = 0;

    /// Allow deleting a reference.
    virtual bool allowDeleteReference(Session& session, const DeleteReferencesItem& item) = 0;

    /// Allow browsing a node.
    virtual bool allowBrowseNode(Session& session, const NodeId& nodeId) = 0;

    /// Allow transfer of a subscription to another session.
    virtual bool allowTransferSubscription(Session& oldSession, Session& newSession) = 0;

    /// Allow insert, replace, update of historical data.
    virtual bool allowHistoryUpdate(
        Session& session,
        const NodeId& nodeId,
        PerformUpdateType performInsertReplace,
        const DataValue& value
    ) = 0;

    /// Allow delete of historical data.
    virtual bool allowHistoryDelete(
        Session& session,
        const NodeId& nodeId,
        DateTime startTimestamp,
        DateTime endTimestamp,
        bool isDeleteModified
    ) = 0;

    UA_AccessControl create() override;
    void clear(UA_AccessControl& ac) noexcept override;
};

/* ----------------------------------- Default access control ----------------------------------- */

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
    explicit AccessControlDefault(bool allowAnonymous = true, std::vector<Login> logins = {});

    Span<UserTokenPolicy> getUserTokenPolicies() override;

    StatusCode activateSession(
        Session& session,
        const EndpointDescription& endpointDescription,
        const ByteString& secureChannelRemoteCertificate,
        const ExtensionObject& userIdentityToken
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
