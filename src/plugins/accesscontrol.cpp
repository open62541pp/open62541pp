#include "open62541pp/AccessControl.h"

#include <cassert>
#include <cstdint>
#include <exception>
#include <functional>  // invoke
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>  // invoke_result_t
#include <utility>  // move

#include "open62541pp/Config.h"
#include "open62541pp/Logger.h"
#include "open62541pp/Server.h"  // getWrapper
#include "open62541pp/Session.h"
#include "open62541pp/Wrapper.h"  // asWrapper, asNative
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/types/DateTime.h"
#include "open62541pp/types/ExtensionObject.h"

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
    const auto message =
        std::string("Exception in access control callback ")
            .append(callbackName)
            .append(": ")
            .append(exceptionMessage);
    log(server, LogLevel::Warning, LogCategory::Server, message);
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
    try {
        auto session = getSession(server, sessionId);
        getAdapter(ac).closeSession(session.value());  // NOLINT(bugprone-unchecked-optional-access)
    } catch (const std::exception& e) {
        logException(server, "closeSession", e.what());
    }
}

static UA_UInt32 getUserRightsMaskNative(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    [[maybe_unused]] void* nodeContext
) {
    return invokeAccessCallback(server, "getUserRightsMask", uint32_t{}, [&] {
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
    return invokeAccessCallback(server, "getUserAccessLevel", uint8_t{}, [&] {
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
        return getAdapter(ac).getUserExecutable(
            session.value(), asWrapperRef<NodeId>(methodId)
        );
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
        return getAdapter(ac).allowDeleteNode(
            session.value(), asWrapperRef<DeleteNodesItem>(item)
        );
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
        return getAdapter(ac).allowTransferSubscription(
            oldSession.value(), newSession.value()
        );
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

UA_AccessControl AccessControlBase::create() {
    UA_AccessControl ac{};
    ac.context = this;
    ac.userTokenPoliciesSize = getUserTokenPolicies().size();
    ac.userTokenPolicies = asNative(getUserTokenPolicies().data());
    ac.activateSession = activateSessionNative;
    ac.closeSession = closeSessionNative;
    ac.getUserRightsMask = getUserRightsMaskNative;
    ac.getUserAccessLevel = getUserAccessLevelNative;
    ac.getUserExecutable = getUserExecutableNative;
    ac.getUserExecutableOnObject = getUserExecutableOnObjectNative;
    ac.allowAddNode = allowAddNodeNative;
    ac.allowAddReference = allowAddReferenceNative;
    ac.allowDeleteNode = allowDeleteNodeNative;
    ac.allowDeleteReference = allowDeleteReferenceNative;
#if UAPP_OPEN62541_VER_GE(1, 1)
    ac.allowBrowseNode = allowBrowseNodeNative;
#endif
#if UAPP_OPEN62541_VER_GE(1, 2) && defined(UA_ENABLE_SUBSCRIPTIONS)
    ac.allowTransferSubscription = allowTransferSubscriptionNative;
#endif
#ifdef UA_ENABLE_HISTORIZING
    ac.allowHistoryUpdateUpdateData = allowHistoryUpdateUpdateDataNative;
    ac.allowHistoryUpdateDeleteRawModified = allowHistoryUpdateDeleteRawModifiedNative;
#endif
    return ac;
}

void AccessControlBase::clear(UA_AccessControl& ac) noexcept {
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

/* ----------------------------------- Default access control ----------------------------------- */

constexpr std::string_view policyIdAnonymous = "open62541-anonymous-policy";
constexpr std::string_view policyIdUsername = "open62541-username-policy";

AccessControlDefault::AccessControlDefault(bool allowAnonymous, std::vector<Login> logins)
    : allowAnonymous_(allowAnonymous),
      logins_(std::move(logins)) {
    const std::string_view issuedTokenType{};
    const std::string_view issuerEndpointUrl{};
    const std::string_view securityPolicyUri{};
    if (allowAnonymous_) {
        userTokenPolicies_.emplace_back(
            policyIdAnonymous,
            UserTokenType::Anonymous,
            issuedTokenType,
            issuerEndpointUrl,
            securityPolicyUri
        );
    }
    if (!logins_.empty()) {
        userTokenPolicies_.emplace_back(
            policyIdUsername,
            UserTokenType::Username,
            issuedTokenType,
            issuerEndpointUrl,
            securityPolicyUri
        );
    }
}

Span<UserTokenPolicy> AccessControlDefault::getUserTokenPolicies() {
    return userTokenPolicies_;
}

StatusCode AccessControlDefault::activateSession(
    [[maybe_unused]] Session& session,
    [[maybe_unused]] const EndpointDescription& endpointDescription,
    [[maybe_unused]] const ByteString& secureChannelRemoteCertificate,
    const ExtensionObject& userIdentityToken
) {
    // https://github.com/open62541/open62541/blob/v1.3.6/plugins/ua_accesscontrol_default.c#L38-L134

    // empty token
    if (userIdentityToken.isEmpty()) {
        if (allowAnonymous_) {
            return UA_STATUSCODE_GOOD;
        }
        return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
    }

    // unknown token type
    if (!userIdentityToken.isDecoded()) {
        return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
    }

    // anonymous login
    if (const auto* token = userIdentityToken.getDecodedData<AnonymousIdentityToken>();
        token != nullptr) {
        if (!allowAnonymous_) {
            return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
        }
        if (token->getPolicyId().empty() || token->getPolicyId() == policyIdAnonymous) {
            return UA_STATUSCODE_GOOD;
        }
        return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
    }

    // username and password
    if (const auto* token = userIdentityToken.getDecodedData<UserNameIdentityToken>();
        token != nullptr) {
        if (token->getPolicyId() != policyIdUsername) {
            return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
        }
        // empty username and password
        if (token->getUserName().empty() && token->getPassword().empty()) {
            return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
        }
        // try to match username / password
        for (const auto& login : logins_) {
            if ((login.username == token->getUserName()) &&
                (login.password == token->getPassword())) {
                return UA_STATUSCODE_GOOD;
            }
        }
        return UA_STATUSCODE_BADUSERACCESSDENIED;
    }

    return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
}

void AccessControlDefault::closeSession([[maybe_unused]] Session& session) {}

Bitmask<WriteMask> AccessControlDefault::getUserRightsMask(
    [[maybe_unused]] Session& session, [[maybe_unused]] const NodeId& nodeId
) {
    return 0xFFFFFFFF;
}

Bitmask<AccessLevel> AccessControlDefault::getUserAccessLevel(
    [[maybe_unused]] Session& session, [[maybe_unused]] const NodeId& nodeId
) {
    return 0xFF;
}

bool AccessControlDefault::getUserExecutable(
    [[maybe_unused]] Session& session, [[maybe_unused]] const NodeId& methodId
) {
    return true;
}

bool AccessControlDefault::getUserExecutableOnObject(
    [[maybe_unused]] Session& session,
    [[maybe_unused]] const NodeId& methodId,
    [[maybe_unused]] const NodeId& objectId
) {
    return true;
}

bool AccessControlDefault::allowAddNode(
    [[maybe_unused]] Session& session, [[maybe_unused]] const AddNodesItem& item
) {
    return true;
}

bool AccessControlDefault::allowAddReference(
    [[maybe_unused]] Session& session, [[maybe_unused]] const AddReferencesItem& item
) {
    return true;
}

bool AccessControlDefault::allowDeleteNode(
    [[maybe_unused]] Session& session, [[maybe_unused]] const DeleteNodesItem& item
) {
    return true;
}

bool AccessControlDefault::allowDeleteReference(
    [[maybe_unused]] Session& session, [[maybe_unused]] const DeleteReferencesItem& item
) {
    return true;
}

bool AccessControlDefault::allowBrowseNode(
    [[maybe_unused]] Session& session, [[maybe_unused]] const NodeId& nodeId
) {
    return true;
}

bool AccessControlDefault::allowTransferSubscription(
    [[maybe_unused]] Session& oldSession, [[maybe_unused]] Session& newSession
) {
    return true;
}

bool AccessControlDefault::allowHistoryUpdate(
    [[maybe_unused]] Session& session,
    [[maybe_unused]] const NodeId& nodeId,
    [[maybe_unused]] PerformUpdateType performInsertReplace,
    [[maybe_unused]] const DataValue& value
) {
    return true;
}

bool AccessControlDefault::allowHistoryDelete(
    [[maybe_unused]] Session& session,
    [[maybe_unused]] const NodeId& nodeId,
    [[maybe_unused]] DateTime startTimestamp,  // NOLINT
    [[maybe_unused]] DateTime endTimestamp,  // NOLINT
    [[maybe_unused]] bool isDeleteModified
) {
    return true;
}

}  // namespace opcua
