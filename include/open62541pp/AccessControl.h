#pragma once

#include <any>
#include <vector>

#include "open62541pp/Auth.h"  // Login
#include "open62541pp/Config.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/ExtensionObject.h"

// forward declare
struct UA_AccessControl;

namespace opcua {

// forward declare
class Server;

/**
 * Access control base class.
 *
 * Used to authenticate sessions and grant access rights accordingly.
 * Custom access control can be implemented by deriving from this class and overwriting the access
 * control callbacks.
 *
 * The `sessionId` and `sessionContext` can originally be both `NULL` in open62541.
 * This is the case when, for example, a MonitoredItem (the underlying Subscription) is detached
 * from its Session but continues to run.
 * This wrapper passes `sessionId` and `sessionContext` by const reference, so they can't be `NULL`.
 * Instead, an empty `NodeId` and `AccessControlBase::SessionContext` will be created.
 *
 * @see UA_AccessControl
 * @see https://www.open62541.org/doc/1.3/plugin_accesscontrol.html
 */
class AccessControlBase {
public:
    /// Arbitrary data attached to a session.
    using SessionContext = std::any;

    AccessControlBase() = default;

    virtual ~AccessControlBase() = default;

    AccessControlBase(const AccessControlBase&) = default;
    AccessControlBase(AccessControlBase&&) noexcept = default;

    AccessControlBase& operator=(const AccessControlBase&) = default;
    AccessControlBase& operator=(AccessControlBase&&) noexcept = default;

    /// Get available user token policies.
    virtual std::vector<UserTokenPolicy> getUserTokenPolicies() noexcept = 0;

    /**
     * Authenticate a session.
     *
     * The session context is attached to the session and later passed into the access control
     * callbacks.
     * The new session is rejected if a status code other than STATUSCODE_GOOD is returned.
     */
    virtual StatusCode activateSession(
        Server& server,
        const EndpointDescription& endpointDescription,
        const ByteString& secureChannelRemoteCertificate,
        const NodeId& sessionId,
        const ExtensionObject& userIdentityToken,
        SessionContext& sessionContext
    ) noexcept = 0;

    /// Deauthenticate a session and cleanup session context.
    virtual void closeSesion(
        Server& server, const NodeId& sessionId, SessionContext& sessionContext
    ) noexcept = 0;

    /// Access control for all nodes.
    virtual uint32_t getUserRightsMask(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& nodeId,
        void* nodeContext
    ) noexcept = 0;

    /// Additional access control for variable nodes.
    virtual uint8_t getUserAccessLevel(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& nodeId,
        void* nodeContext
    ) noexcept = 0;

    /// Additional access control for method nodes.
    virtual bool getUserExecutable(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& methodId,
        void* methodContext
    ) noexcept = 0;

    /// Additional access control for calling a method node in the context of a specific object.
    virtual bool getUserExecutableOnObject(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& methodId,
        void* methodContext,
        const NodeId& objectId,
        void* objectContext
    ) noexcept = 0;

    /// Allow adding a node.
    virtual bool allowAddNode(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const AddNodesItem& item
    ) noexcept = 0;

    /// Allow adding a reference.
    virtual bool allowAddReference(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const AddReferencesItem& item
    ) noexcept = 0;

    /// Allow deleting a node.
    virtual bool allowDeleteNode(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const DeleteNodesItem& item
    ) noexcept = 0;

    /// Allow deleting a reference.
    virtual bool allowDeleteReference(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const DeleteReferencesItem& item
    ) noexcept = 0;

    /// Allow browsing a node.
    virtual bool allowBrowseNode(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& nodeId,
        void* nodeContext
    ) noexcept = 0;

#ifdef UA_ENABLE_SUBSCRIPTIONS
    /// Allow transfer of a subscription to another session.
    virtual bool allowTransferSubscription(
        Server& server,
        const NodeId& oldSessionId,
        SessionContext& oldSessionContext,
        const NodeId& newSessionId,
        SessionContext& newSessionContext
    ) noexcept = 0;
#endif

#ifdef UA_ENABLE_HISTORIZING
    /// Allow insert, replace, update of historical data.
    virtual bool allowHistoryUpdate(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& nodeId,
        PerformUpdateType performInsertReplace,  // TODO
        const UA_DataValue& value
    ) noexcept = 0;

    /// Allow delete of historical data.
    virtual bool allowHistoryDelete(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& nodeId,
        DateTime startTimestamp,
        DateTime endTimestamp,
        bool isDeleteModified
    ) noexcept = 0;
#endif
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

    std::vector<UserTokenPolicy> getUserTokenPolicies() noexcept override;

    StatusCode activateSession(
        Server& server,
        const EndpointDescription& endpointDescription,
        const ByteString& secureChannelRemoteCertificate,
        const NodeId& sessionId,
        const ExtensionObject& userIdentityToken,
        SessionContext& sessionContext
    ) noexcept override;

    void closeSesion(
        Server& server, const NodeId& sessionId, SessionContext& sessionContext
    ) noexcept override;

    uint32_t getUserRightsMask(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& nodeId,
        void* nodeContext
    ) noexcept override;

    uint8_t getUserAccessLevel(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& nodeId,
        void* nodeContext
    ) noexcept override;

    bool getUserExecutable(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& methodId,
        void* methodContext
    ) noexcept override;

    bool getUserExecutableOnObject(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& methodId,
        void* methodContext,
        const NodeId& objectId,
        void* objectContext
    ) noexcept override;

    bool allowAddNode(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const AddNodesItem& item
    ) noexcept override;

    bool allowAddReference(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const AddReferencesItem& item
    ) noexcept override;

    bool allowDeleteNode(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const DeleteNodesItem& item
    ) noexcept override;

    bool allowDeleteReference(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const DeleteReferencesItem& item
    ) noexcept override;

    bool allowBrowseNode(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& nodeId,
        void* nodeContext
    ) noexcept override;

#ifdef UA_ENABLE_SUBSCRIPTIONS
    bool allowTransferSubscription(
        Server& server,
        const NodeId& oldSessionId,
        SessionContext& oldSessionContext,
        const NodeId& newSessionId,
        SessionContext& newSessionContext
    ) noexcept override;
#endif

#ifdef UA_ENABLE_HISTORIZING
    bool allowHistoryUpdate(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& nodeId,
        PerformUpdateType performInsertReplace,  // TODO
        const UA_DataValue& value
    ) noexcept override;

    bool allowHistoryDelete(
        Server& server,
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& nodeId,
        DateTime startTimestamp,
        DateTime endTimestamp,
        bool isDeleteModified
    ) noexcept override;
#endif

private:
    bool allowAnonymous_;
    std::vector<Login> logins_;
};

}  // namespace opcua
