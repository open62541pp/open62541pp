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

    explicit AccessControlBase(Server& server);

    virtual ~AccessControlBase();

    AccessControlBase(const AccessControlBase& other) = delete;
    AccessControlBase(AccessControlBase&& other) noexcept = delete;

    AccessControlBase& operator=(const AccessControlBase& other) = delete;
    AccessControlBase& operator=(AccessControlBase&& other) noexcept = delete;

    /// Get available user token policies.
    virtual std::vector<UserTokenPolicy> getUserTokenPolicies() noexcept = 0;

    /**
     * Authenticate a session.
     *
     * The session context is attached to the session and later passed into the access control
     * callbacks.
     * The new session is rejected if a status code other than UA_STATUSCODE_GOOD is returned.
     */
    virtual UA_StatusCode activateSession(
        const EndpointDescription& endpointDescription,
        const ByteString& secureChannelRemoteCertificate,
        const NodeId& sessionId,
        const ExtensionObject& userIdentityToken,
        SessionContext& sessionContext
    ) noexcept = 0;

    /// Deauthenticate a session and cleanup session context.
    virtual void closeSesion(const NodeId& sessionId, SessionContext& sessionContext) noexcept = 0;

    /// Access control for all nodes.
    virtual uint32_t getUserRightsMask(
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& nodeId,
        void* nodeContext
    ) noexcept = 0;

    /// Additional access control for variable nodes.
    virtual uint8_t getUserAccessLevel(
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& nodeId,
        void* nodeContext
    ) noexcept = 0;

    /// Additional access control for method nodes.
    virtual bool getUserExecutable(
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& methodId,
        void* methodContext
    ) noexcept = 0;

    /// Additional access control for calling a method node in the context of a specific object.
    virtual bool getUserExecutableOnObject(
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& methodId,
        void* methodContext,
        const NodeId& objectId,
        void* objectContext
    ) noexcept = 0;

    /// Allow adding a node.
    virtual bool allowAddNode(
        const NodeId& sessionId, SessionContext& sessionContext, const AddNodesItem& item
    ) noexcept = 0;

    /// Allow adding a reference.
    virtual bool allowAddReference(
        const NodeId& sessionId, SessionContext& sessionContext, const AddReferencesItem& item
    ) noexcept = 0;

    /// Allow deleting a node.
    virtual bool allowDeleteNode(
        const NodeId& sessionId, SessionContext& sessionContext, const DeleteNodesItem& item
    ) noexcept = 0;

    /// Allow deleting a reference.
    virtual bool allowDeleteReference(
        const NodeId& sessionId, SessionContext& sessionContext, const DeleteReferencesItem& item
    ) noexcept = 0;

    /// Allow browsing a node.
    virtual bool allowBrowseNode(
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& nodeId,
        void* nodeContext
    ) noexcept = 0;

#ifdef UA_ENABLE_SUBSCRIPTIONS
    /// Allow transfer of a subscription to another session.
    virtual bool allowTransferSubscription(
        const NodeId& oldSessionId,
        SessionContext& oldSessionContext,
        const NodeId& newSessionId,
        SessionContext& newSessionContext
    ) noexcept = 0;
#endif

#ifdef UA_ENABLE_HISTORIZING
    /// Allow insert, replace, update of historical data.
    virtual bool allowHistoryUpdate(
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& nodeId,
        PerformUpdateType performInsertReplace,  // TODO
        const UA_DataValue& value
    ) noexcept = 0;

    /// Allow delete of historical data.
    virtual bool allowHistoryDelete(
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& nodeId,
        DateTime startTimestamp,
        DateTime endTimestamp,
        bool isDeleteModified
    ) noexcept = 0;
#endif

protected:
    Server& getServer() noexcept;
    const Server& getServer() const noexcept;

private:
    Server& server_;
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
    explicit AccessControlDefault(
        Server& server, bool allowAnonymous = true, std::vector<Login> logins = {}
    );

    std::vector<UserTokenPolicy> getUserTokenPolicies() noexcept override;

    UA_StatusCode activateSession(
        const EndpointDescription& endpointDescription,
        const ByteString& secureChannelRemoteCertificate,
        const NodeId& sessionId,
        const ExtensionObject& userIdentityToken,
        SessionContext& sessionContext
    ) noexcept override;

    void closeSesion(const NodeId& sessionId, SessionContext& sessionContext) noexcept override;

    uint32_t getUserRightsMask(
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& nodeId,
        void* nodeContext
    ) noexcept override;

    uint8_t getUserAccessLevel(
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& nodeId,
        void* nodeContext
    ) noexcept override;

    bool getUserExecutable(
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& methodId,
        void* methodContext
    ) noexcept override;

    bool getUserExecutableOnObject(
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& methodId,
        void* methodContext,
        const NodeId& objectId,
        void* objectContext
    ) noexcept override;

    bool allowAddNode(
        const NodeId& sessionId, SessionContext& sessionContext, const AddNodesItem& item
    ) noexcept override;

    bool allowAddReference(
        const NodeId& sessionId, SessionContext& sessionContext, const AddReferencesItem& item
    ) noexcept override;

    bool allowDeleteNode(
        const NodeId& sessionId, SessionContext& sessionContext, const DeleteNodesItem& item
    ) noexcept override;

    bool allowDeleteReference(
        const NodeId& sessionId, SessionContext& sessionContext, const DeleteReferencesItem& item
    ) noexcept override;

    bool allowBrowseNode(
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& nodeId,
        void* nodeContext
    ) noexcept override;

#ifdef UA_ENABLE_SUBSCRIPTIONS
    bool allowTransferSubscription(
        const NodeId& oldSessionId,
        SessionContext& oldSessionContext,
        const NodeId& newSessionId,
        SessionContext& newSessionContext
    ) noexcept override;
#endif

#ifdef UA_ENABLE_HISTORIZING
    bool allowHistoryUpdate(
        const NodeId& sessionId,
        SessionContext& sessionContext,
        const NodeId& nodeId,
        PerformUpdateType performInsertReplace,  // TODO
        const UA_DataValue& value
    ) noexcept override;

    bool allowHistoryDelete(
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
