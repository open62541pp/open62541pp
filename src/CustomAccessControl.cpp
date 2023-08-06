#include "CustomAccessControl.h"

#include <cassert>

#include "open62541pp/AccessControl.h"
#include "open62541pp/Config.h"
#include "open62541pp/Server.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/DateTime.h"

#include "open62541_impl.h"

namespace opcua {

/* -------------------------------------- Native callbacks -------------------------------------- */

inline static CustomAccessControl::Context& getContext(UA_AccessControl* ac) noexcept {
    assert(ac != nullptr);  // NOLINT
    assert(ac->context != nullptr);  // NOLINT
    auto& context = *static_cast<CustomAccessControl::Context*>(ac->context);
    assert(context.accessControl != nullptr);  // NOLINT
    return context;
}

inline static Server& getServer(UA_AccessControl* ac) noexcept {
    return getContext(ac).server;
}

inline static AccessControlBase& getAccessControl(UA_AccessControl* ac) noexcept {
    return *getContext(ac).accessControl;
}

template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
inline static const WrapperType& asWrapperRef(const NativeType* nativePtr) {
    static const WrapperType empty;
    return nativePtr == nullptr ? empty : asWrapper<WrapperType>(*nativePtr);
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
    return getAccessControl(ac).activateSession(
        getServer(ac),
        asWrapperRef<NodeId>(sessionId),
        asWrapperRef<EndpointDescription>(endpointDescription),
        asWrapperRef<ByteString>(secureChannelRemoteCertificate),
        asWrapperRef<ExtensionObject>(userIdentityToken)
    );
}

static void closeSession(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext
) {
    return getAccessControl(ac).closeSession(getServer(ac), asWrapperRef<NodeId>(sessionId));
}

static UA_UInt32 getUserRightsMask(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    [[maybe_unused]] void* nodeContext
) {
    return getAccessControl(ac).getUserRightsMask(
        getServer(ac), asWrapperRef<NodeId>(sessionId), asWrapperRef<NodeId>(nodeId)
    );
}

static UA_Byte getUserAccessLevel(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    [[maybe_unused]] void* nodeContext
) {
    return getAccessControl(ac).getUserAccessLevel(
        getServer(ac), asWrapperRef<NodeId>(sessionId), asWrapperRef<NodeId>(nodeId)
    );
}

static UA_Boolean getUserExecutable(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* methodId,
    [[maybe_unused]] void* methodContext
) {
    return getAccessControl(ac).getUserExecutable(
        getServer(ac), asWrapperRef<NodeId>(sessionId), asWrapperRef<NodeId>(methodId)
    );
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
    return getAccessControl(ac).getUserExecutableOnObject(
        getServer(ac),
        asWrapperRef<NodeId>(sessionId),
        asWrapperRef<NodeId>(methodId),
        asWrapperRef<NodeId>(objectId)
    );
}

static UA_Boolean allowAddNode(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_AddNodesItem* item
) {
    return getAccessControl(ac).allowAddNode(
        getServer(ac), asWrapperRef<NodeId>(sessionId), asWrapperRef<AddNodesItem>(item)
    );
}

static UA_Boolean allowAddReference(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_AddReferencesItem* item
) {
    return getAccessControl(ac).allowAddReference(
        getServer(ac), asWrapperRef<NodeId>(sessionId), asWrapperRef<AddReferencesItem>(item)
    );
}

static UA_Boolean allowDeleteNode(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_DeleteNodesItem* item
) {
    return getAccessControl(ac).allowDeleteNode(
        getServer(ac), asWrapperRef<NodeId>(sessionId), asWrapperRef<DeleteNodesItem>(item)
    );
}

static UA_Boolean allowDeleteReference(
    [[maybe_unused]] UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_DeleteReferencesItem* item
) {
    return getAccessControl(ac).allowDeleteReference(
        getServer(ac), asWrapperRef<NodeId>(sessionId), asWrapperRef<DeleteReferencesItem>(item)
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
    return getAccessControl(ac).allowBrowseNode(
        getServer(ac), asWrapperRef<NodeId>(sessionId), asWrapperRef<NodeId>(nodeId)
    );
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
    return getAccessControl(ac).allowTransferSubscription(
        getServer(ac), asWrapperRef<NodeId>(oldSessionId), asWrapperRef<NodeId>(newSessionId)
    );
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
    return getAccessControl(ac).allowHistoryUpdate(
        getServer(ac),
        asWrapperRef<NodeId>(sessionId),
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
    return getAccessControl(ac).allowHistoryDelete(
        getServer(ac),
        asWrapperRef<NodeId>(sessionId),
        asWrapperRef<NodeId>(nodeId),
        DateTime(startTimestamp),
        DateTime(endTimestamp),
        isDeleteModified
    );
}
#endif

/* ---------------------------------------------------------------------------------------------- */

CustomAccessControl::CustomAccessControl(Server& server, UA_AccessControl& native)
    : native_{native},
      context_{server, nullptr, {}} {}

void CustomAccessControl::setAccessControl() {
    if (context_.accessControl == nullptr) {
        return;
    }
#if UAPP_OPEN62541_VER_GE(1, 1)
    if (native_.clear != nullptr) {
        native_.clear(&native_);
    }
#else
    if (native_.deleteMembers != nullptr) {
        native_.deleteMembers(&native_);
    }
#endif

    native_ = UA_AccessControl{};
    native_.context = &context_;
    native_.userTokenPoliciesSize = context_.userTokenPolicies.size();
    native_.userTokenPolicies = asNative(context_.userTokenPolicies.data());
    native_.activateSession = activateSession;
    native_.closeSession = closeSession;
    native_.getUserRightsMask = getUserRightsMask;
    native_.getUserAccessLevel = getUserAccessLevel;
    native_.getUserExecutable = getUserExecutable;
    native_.getUserExecutableOnObject = getUserExecutableOnObject;
    native_.allowAddNode = allowAddNode;
    native_.allowAddReference = allowAddReference;
    native_.allowDeleteNode = allowDeleteNode;
    native_.allowDeleteReference = allowDeleteReference;
#if UAPP_OPEN62541_VER_GE(1, 1)
    native_.allowBrowseNode = allowBrowseNode;
#endif
#if UAPP_OPEN62541_VER_GE(1, 2) && defined(UA_ENABLE_SUBSCRIPTIONS)
    native_.allowTransferSubscription = allowTransferSubscription;
#endif
#ifdef UA_ENABLE_HISTORIZING
    native_.allowHistoryUpdateUpdateData = allowHistoryUpdateUpdateData;
    native_.allowHistoryUpdateDeleteRawModified = allowHistoryUpdateDeleteRawModified;
#endif
}

void CustomAccessControl::setAccessControl(AccessControlBase& accessControl) {
    context_.accessControl = &accessControl;
    context_.userTokenPolicies = accessControl.getUserTokenPolicies();
    setAccessControl();
}

}  // namespace opcua
