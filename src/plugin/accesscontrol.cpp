#include "open62541pp/plugin/accesscontrol.hpp"

#include <cassert>
#include <exception>
#include <functional>  // invoke
#include <optional>
#include <string_view>
#include <type_traits>  // invoke_result_t

#include "open62541pp/config.hpp"
#include "open62541pp/plugin/log.hpp"
#include "open62541pp/server.hpp"  // getWrapper
#include "open62541pp/wrapper.hpp"  // asWrapper, asNative

namespace opcua {

inline static AccessControlBase& getAdapter(UA_AccessControl* ac) {
    assert(ac != nullptr);
    assert(ac->context != nullptr);
    return *static_cast<AccessControlBase*>(ac->context);
}

template <typename WrapperType, typename NativeType = typename WrapperType::NativeType>
inline static const WrapperType& asWrapperRef(const NativeType* nativePtr) {
    static const WrapperType empty;
    return nativePtr == nullptr ? empty : asWrapper<WrapperType>(*nativePtr);
}

inline static std::optional<Session> getSession(
    UA_Server* server, const UA_NodeId* sessionId
) noexcept {
    auto* wrapper = detail::getWrapper(server);
    if (wrapper == nullptr) {
        return std::nullopt;
    }
    return Session(*wrapper, asWrapperRef<NodeId>(sessionId));
}

static void logException(
    UA_Server* server, std::string_view callbackName, std::string_view exceptionMessage
) {
    // NOLINTNEXTLINE
    UA_LOG_WARNING(
        detail::getLogger(server),
        UA_LOGCATEGORY_SERVER,
        "Exception in access control callback %.*s: %.*s",
        static_cast<int>(callbackName.size()),
        callbackName.data(),
        static_cast<int>(exceptionMessage.size()),
        exceptionMessage.data()
    );
}

template <typename F, typename ReturnType = std::invoke_result_t<F>>
inline static auto invokeAccessCallback(
    UA_Server* server, std::string_view callbackName, ReturnType returnOnException, F&& fn
) noexcept {
    try {
        return std::invoke(std::forward<F>(fn));
    } catch (const std::exception& e) {
        logException(server, callbackName, e.what());
        return returnOnException;
    }
}

static UA_StatusCode activateSessionNative(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_EndpointDescription* endpointDescription,
    const UA_ByteString* secureChannelRemoteCertificate,
    const UA_NodeId* sessionId,
    const UA_ExtensionObject* userIdentityToken,
    [[maybe_unused]] void** sessionContext
) {
    return invokeAccessCallback(server, "activateSession", UA_STATUSCODE_BADINTERNALERROR, [&] {
        auto session = getSession(server, sessionId);
        return getAdapter(ac)
            .activateSession(
                session.value(),
                asWrapperRef<EndpointDescription>(endpointDescription),
                asWrapperRef<ByteString>(secureChannelRemoteCertificate),
                asWrapperRef<ExtensionObject>(userIdentityToken)
            )
            .get();
    });
}

static void closeSessionNative(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext
) {
    invokeAccessCallback(server, "activateSession", UA_STATUSCODE_GOOD, [&] {
        auto session = getSession(server, sessionId);
        getAdapter(ac).closeSession(session.value());  // NOLINT(bugprone-unchecked-optional-access)
        return UA_STATUSCODE_GOOD;
    });
}

static UA_UInt32 getUserRightsMaskNative(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    [[maybe_unused]] void* nodeContext
) {
    return invokeAccessCallback(server, "getUserRightsMask", UA_UInt32{}, [&] {
        auto session = getSession(server, sessionId);
        return getAdapter(ac)
            .getUserRightsMask(session.value(), asWrapperRef<NodeId>(nodeId))
            .get();
    });
}

static UA_Byte getUserAccessLevelNative(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    [[maybe_unused]] void* nodeContext
) {
    return invokeAccessCallback(server, "getUserAccessLevel", UA_Byte{}, [&] {
        auto session = getSession(server, sessionId);
        return getAdapter(ac)
            .getUserAccessLevel(session.value(), asWrapperRef<NodeId>(nodeId))
            .get();
    });
}

static UA_Boolean getUserExecutableNative(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* methodId,
    [[maybe_unused]] void* methodContext
) {
    return invokeAccessCallback(server, "getUserExecutable", false, [&] {
        auto session = getSession(server, sessionId);
        return getAdapter(ac).getUserExecutable(session.value(), asWrapperRef<NodeId>(methodId));
    });
}

static UA_Boolean getUserExecutableOnObjectNative(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* methodId,
    [[maybe_unused]] void* methodContext,
    const UA_NodeId* objectId,
    [[maybe_unused]] void* objectContext
) {
    return invokeAccessCallback(server, "getUserExecutableOnObject", false, [&] {
        auto session = getSession(server, sessionId);
        return getAdapter(ac).getUserExecutableOnObject(
            session.value(), asWrapperRef<NodeId>(methodId), asWrapperRef<NodeId>(objectId)
        );
    });
}

static UA_Boolean allowAddNodeNative(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_AddNodesItem* item
) {
    return invokeAccessCallback(server, "allowAddNode", false, [&] {
        auto session = getSession(server, sessionId);
        return getAdapter(ac).allowAddNode(session.value(), asWrapperRef<AddNodesItem>(item));
    });
}

static UA_Boolean allowAddReferenceNative(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_AddReferencesItem* item
) {
    return invokeAccessCallback(server, "allowAddReference", false, [&] {
        auto session = getSession(server, sessionId);
        return getAdapter(ac).allowAddReference(
            session.value(), asWrapperRef<AddReferencesItem>(item)
        );
    });
}

static UA_Boolean allowDeleteNodeNative(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_DeleteNodesItem* item
) {
    return invokeAccessCallback(server, "allowDeleteNode", false, [&] {
        auto session = getSession(server, sessionId);
        return getAdapter(ac).allowDeleteNode(session.value(), asWrapperRef<DeleteNodesItem>(item));
    });
}

static UA_Boolean allowDeleteReferenceNative(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_DeleteReferencesItem* item
) {
    return invokeAccessCallback(server, "allowDeleteReference", false, [&] {
        auto session = getSession(server, sessionId);
        return getAdapter(ac).allowDeleteReference(
            session.value(), asWrapperRef<DeleteReferencesItem>(item)
        );
    });
}

[[maybe_unused]] static UA_Boolean allowBrowseNodeNative(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    [[maybe_unused]] void* nodeContext
) {
    return invokeAccessCallback(server, "allowBrowseNode", false, [&] {
        auto session = getSession(server, sessionId);
        return getAdapter(ac).allowBrowseNode(session.value(), asWrapperRef<NodeId>(nodeId));
    });
}

#ifdef UA_ENABLE_SUBSCRIPTIONS
[[maybe_unused]] static UA_Boolean allowTransferSubscriptionNative(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* oldSessionId,
    [[maybe_unused]] void* oldSessionContext,
    const UA_NodeId* newSessionId,
    [[maybe_unused]] void* newSessionContext
) {
    return invokeAccessCallback(server, "allowTransferSubscription", false, [&] {
        auto oldSession = getSession(server, oldSessionId);
        auto newSession = getSession(server, newSessionId);
        return getAdapter(ac).allowTransferSubscription(oldSession.value(), newSession.value());
    });
}
#endif

#ifdef UA_ENABLE_HISTORIZING
static UA_Boolean allowHistoryUpdateUpdateDataNative(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    UA_PerformUpdateType performInsertReplace,
    const UA_DataValue* value
) {
    return invokeAccessCallback(server, "allowHistoryUpdate", false, [&] {
        auto session = getSession(server, sessionId);
        return getAdapter(ac).allowHistoryUpdate(
            session.value(),
            asWrapperRef<NodeId>(nodeId),
            static_cast<PerformUpdateType>(performInsertReplace),
            asWrapperRef<DataValue>(value)
        );
    });
}

static UA_Boolean allowHistoryUpdateDeleteRawModifiedNative(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    UA_DateTime startTimestamp,
    UA_DateTime endTimestamp,
    bool isDeleteModified
) {
    return invokeAccessCallback(server, "allowHistoryDelete", false, [&] {
        auto session = getSession(server, sessionId);
        return getAdapter(ac).allowHistoryDelete(
            session.value(),
            asWrapperRef<NodeId>(nodeId),
            DateTime(startTimestamp),
            DateTime(endTimestamp),
            isDeleteModified
        );
    });
}
#endif

inline static auto& clearFunction(UA_AccessControl& ac) noexcept {
#if UAPP_OPEN62541_VER_GE(1, 1)
    return ac.clear;
#else
    return ac.deleteMembers;
#endif
}

UA_AccessControl AccessControlBase::create(bool ownsAdapter) {
    UA_AccessControl native{};
    native.context = this;
    if (ownsAdapter) {
        clearFunction(native) = [](UA_AccessControl* ac) {
            if (ac != nullptr) {
                delete static_cast<AccessControlBase*>(ac->context);  // NOLINT
                ac->context = nullptr;
            }
        };
    }
    native.userTokenPoliciesSize = getUserTokenPolicies().size();
    native.userTokenPolicies = asNative(getUserTokenPolicies().data());
    native.activateSession = activateSessionNative;
    native.closeSession = closeSessionNative;
    native.getUserRightsMask = getUserRightsMaskNative;
    native.getUserAccessLevel = getUserAccessLevelNative;
    native.getUserExecutable = getUserExecutableNative;
    native.getUserExecutableOnObject = getUserExecutableOnObjectNative;
    native.allowAddNode = allowAddNodeNative;
    native.allowAddReference = allowAddReferenceNative;
    native.allowDeleteNode = allowDeleteNodeNative;
    native.allowDeleteReference = allowDeleteReferenceNative;
#if UAPP_OPEN62541_VER_GE(1, 1)
    native.allowBrowseNode = allowBrowseNodeNative;
#endif
#if UAPP_OPEN62541_VER_GE(1, 2) && defined(UA_ENABLE_SUBSCRIPTIONS)
    native.allowTransferSubscription = allowTransferSubscriptionNative;
#endif
#ifdef UA_ENABLE_HISTORIZING
    native.allowHistoryUpdateUpdateData = allowHistoryUpdateUpdateDataNative;
    native.allowHistoryUpdateDeleteRawModified = allowHistoryUpdateDeleteRawModifiedNative;
#endif
    return native;
}

void AccessControlBase::clear(UA_AccessControl& ac) const noexcept {
    if (clearFunction(ac) != nullptr) {
        clearFunction(ac)(&ac);
    }
    ac = UA_AccessControl{};
}

}  // namespace opcua
