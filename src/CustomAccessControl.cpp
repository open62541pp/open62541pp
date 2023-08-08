#include "CustomAccessControl.h"

#include <cassert>
#include <utility>  // move

#include "open62541pp/AccessControl.h"
#include "open62541pp/Config.h"
#include "open62541pp/Server.h"
#include "open62541pp/Session.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/DateTime.h"
#include "open62541pp/types/NodeId.h"

#include "open62541_impl.h"

namespace opcua {

/* -------------------------------------- Native callbacks -------------------------------------- */

inline static CustomAccessControl& getContext(UA_AccessControl* ac) noexcept {
    assert(ac != nullptr);  // NOLINT
    assert(ac->context != nullptr);  // NOLINT
    return *static_cast<CustomAccessControl*>(ac->context);
}

inline static Server& getServer(UA_AccessControl* ac) noexcept {
    return getContext(ac).getServer();
}

template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
inline static const WrapperType& asWrapperRef(const NativeType* nativePtr) {
    static const WrapperType empty;
    return nativePtr == nullptr ? empty : asWrapper<WrapperType>(*nativePtr);
}

inline static Session getSession(UA_AccessControl* ac, const UA_NodeId* sessionId) noexcept {
    // session constructed with every call to access control
    // future optimization: store session object in sessionContext
    return {getServer(ac), asWrapperRef<NodeId>(sessionId)};
}

inline static AccessControlBase& getAccessControl(UA_AccessControl* ac) noexcept {
    auto* accessControl = getContext(ac).getAccessControl();
    assert(accessControl != nullptr);  // NOLINT
    return *accessControl;
}

static UA_StatusCode activateSession(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_EndpointDescription* endpointDescription,
    const UA_ByteString* secureChannelRemoteCertificate,
    const UA_NodeId* sessionId,
    const UA_ExtensionObject* userIdentityToken,
    [[maybe_unused]] void** sessionContext
) {
    auto session = getSession(ac, sessionId);
    const auto status = getAccessControl(ac).activateSession(
        session,
        asWrapperRef<EndpointDescription>(endpointDescription),
        asWrapperRef<ByteString>(secureChannelRemoteCertificate),
        asWrapperRef<ExtensionObject>(userIdentityToken)
    );
    if (status.isGood()) {
        getContext(ac).onSessionActivated(asWrapperRef<NodeId>(sessionId));
    }
    return status.get();
}

static void closeSession(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext
) {
    auto session = getSession(ac, sessionId);
    getAccessControl(ac).closeSession(session);
    getContext(ac).onSessionClosed(asWrapperRef<NodeId>(sessionId));
}

static UA_UInt32 getUserRightsMask(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    [[maybe_unused]] void* nodeContext
) {
    auto session = getSession(ac, sessionId);
    return getAccessControl(ac).getUserRightsMask(session, asWrapperRef<NodeId>(nodeId));
}

static UA_Byte getUserAccessLevel(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    [[maybe_unused]] void* nodeContext
) {
    auto session = getSession(ac, sessionId);
    return getAccessControl(ac).getUserAccessLevel(session, asWrapperRef<NodeId>(nodeId));
}

static UA_Boolean getUserExecutable(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* methodId,
    [[maybe_unused]] void* methodContext
) {
    auto session = getSession(ac, sessionId);
    return getAccessControl(ac).getUserExecutable(session, asWrapperRef<NodeId>(methodId));
}

static UA_Boolean getUserExecutableOnObject(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* methodId,
    [[maybe_unused]] void* methodContext,
    const UA_NodeId* objectId,
    [[maybe_unused]] void* objectContext
) {
    auto session = getSession(ac, sessionId);
    return getAccessControl(ac).getUserExecutableOnObject(
        session, asWrapperRef<NodeId>(methodId), asWrapperRef<NodeId>(objectId)
    );
}

static UA_Boolean allowAddNode(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_AddNodesItem* item
) {
    auto session = getSession(ac, sessionId);
    return getAccessControl(ac).allowAddNode(session, asWrapperRef<AddNodesItem>(item));
}

static UA_Boolean allowAddReference(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_AddReferencesItem* item
) {
    auto session = getSession(ac, sessionId);
    return getAccessControl(ac).allowAddReference(session, asWrapperRef<AddReferencesItem>(item));
}

static UA_Boolean allowDeleteNode(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_DeleteNodesItem* item
) {
    auto session = getSession(ac, sessionId);
    return getAccessControl(ac).allowDeleteNode(session, asWrapperRef<DeleteNodesItem>(item));
}

static UA_Boolean allowDeleteReference(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_DeleteReferencesItem* item
) {
    auto session = getSession(ac, sessionId);
    return getAccessControl(ac).allowDeleteReference(
        session, asWrapperRef<DeleteReferencesItem>(item)
    );
}

[[maybe_unused]] static UA_Boolean allowBrowseNode(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    [[maybe_unused]] void* nodeContext
) {
    auto session = getSession(ac, sessionId);
    return getAccessControl(ac).allowBrowseNode(session, asWrapperRef<NodeId>(nodeId));
}

#ifdef UA_ENABLE_SUBSCRIPTIONS
[[maybe_unused]] static UA_Boolean allowTransferSubscription(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* oldSessionId,
    [[maybe_unused]] void* oldSessionContext,
    const UA_NodeId* newSessionId,
    [[maybe_unused]] void* newSessionContext
) {
    auto oldSession = getSession(ac, oldSessionId);
    auto newSession = getSession(ac, newSessionId);
    return getAccessControl(ac).allowTransferSubscription(oldSession, newSession);
}
#endif

#ifdef UA_ENABLE_HISTORIZING
static UA_Boolean allowHistoryUpdateUpdateData(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    UA_PerformUpdateType performInsertReplace,
    const UA_DataValue* value
) {
    auto session = getSession(ac, sessionId);
    return getAccessControl(ac).allowHistoryUpdate(
        session,
        asWrapperRef<NodeId>(nodeId),
        static_cast<PerformUpdateType>(performInsertReplace),
        asWrapperRef<DataValue>(value)
    );
}

static UA_Boolean allowHistoryUpdateDeleteRawModified(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    UA_DateTime startTimestamp,
    UA_DateTime endTimestamp,
    bool isDeleteModified
) {
    auto session = getSession(ac, sessionId);
    return getAccessControl(ac).allowHistoryDelete(
        session,
        asWrapperRef<NodeId>(nodeId),
        DateTime(startTimestamp),
        DateTime(endTimestamp),
        isDeleteModified
    );
}
#endif

/* ---------------------------------------------------------------------------------------------- */

namespace detail {

void clearUaAccessControl(UA_AccessControl& ac) noexcept {
#if UAPP_OPEN62541_VER_GE(1, 1)
    if (ac.clear != nullptr) {
        ac.clear(&ac);
    }
#else
    if (ac.deleteMembers != nullptr) {
        ac.deleteMembers(&ac);
    }
#endif
    ac = UA_AccessControl{};
}

}  // namespace detail

CustomAccessControl::CustomAccessControl(Server& server)
    : server_(server),
      accessControl_{nullptr} {}

void CustomAccessControl::setAccessControl() {
    if (getAccessControl() == nullptr) {
        return;
    }
    auto* config = UA_Server_getConfig(getServer().handle());
    assert(config != nullptr);
    auto& ac = config->accessControl;

    detail::clearUaAccessControl(ac);

    ac = UA_AccessControl{};
    ac.context = this;
    ac.userTokenPoliciesSize = userTokenPolicies_.size();
    ac.userTokenPolicies = asNative(userTokenPolicies_.data());
    ac.activateSession = activateSession;
    ac.closeSession = closeSession;
    ac.getUserRightsMask = getUserRightsMask;
    ac.getUserAccessLevel = getUserAccessLevel;
    ac.getUserExecutable = getUserExecutable;
    ac.getUserExecutableOnObject = getUserExecutableOnObject;
    ac.allowAddNode = allowAddNode;
    ac.allowAddReference = allowAddReference;
    ac.allowDeleteNode = allowDeleteNode;
    ac.allowDeleteReference = allowDeleteReference;
#if UAPP_OPEN62541_VER_GE(1, 1)
    ac.allowBrowseNode = allowBrowseNode;
#endif
#if UAPP_OPEN62541_VER_GE(1, 2) && defined(UA_ENABLE_SUBSCRIPTIONS)
    ac.allowTransferSubscription = allowTransferSubscription;
#endif
#ifdef UA_ENABLE_HISTORIZING
    ac.allowHistoryUpdateUpdateData = allowHistoryUpdateUpdateData;
    ac.allowHistoryUpdateDeleteRawModified = allowHistoryUpdateDeleteRawModified;
#endif
}

void CustomAccessControl::setAccessControl(AccessControlBase& accessControl) {
    userTokenPolicies_ = accessControl.getUserTokenPolicies();
    accessControl_ = &accessControl;
    setAccessControl();
}

void CustomAccessControl::setAccessControl(std::unique_ptr<AccessControlBase> accessControl) {
    userTokenPolicies_ = accessControl->getUserTokenPolicies();
    accessControl_ = std::move(accessControl);
    setAccessControl();
}

void CustomAccessControl::onSessionActivated(const NodeId& sessionId) {
    sessionIds_.insert(sessionId);
}

void CustomAccessControl::onSessionClosed(const NodeId& sessionId) {
    sessionIds_.erase(sessionId);
}

std::vector<Session> CustomAccessControl::getSessions() const {
    std::vector<Session> result;
    for (auto id : sessionIds_) {
        result.emplace_back(server_, std::move(id));
    }
    return result;
}

Server& CustomAccessControl::getServer() noexcept {
    return server_;
}

AccessControlBase* CustomAccessControl::getAccessControl() noexcept {
    if (auto* ptr = std::get_if<0>(&accessControl_); ptr != nullptr) {
        return *ptr;
    }
    if (auto* ptr = std::get_if<1>(&accessControl_); ptr != nullptr) {
        return ptr->get();
    }
    return nullptr;
}

}  // namespace opcua
