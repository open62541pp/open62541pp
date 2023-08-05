#include "open62541pp/AccessControl.h"

#include <cassert>
#include <utility>  // move

#include "open62541pp/Server.h"
#include "open62541pp/detail/helper.h"

#include "open62541_impl.h"

namespace opcua {

/* ----------------------------------- Default access control ----------------------------------- */

constexpr std::string_view policyIdAnonymous = "open62541-anonymous-policy";
constexpr std::string_view policyIdUsername = "open62541-username-policy";

AccessControlDefault::AccessControlDefault(bool allowAnonymous, std::vector<Login> logins)
    : allowAnonymous_(allowAnonymous),
      logins_(std::move(logins)) {}

std::vector<UserTokenPolicy> AccessControlDefault::getUserTokenPolicies() noexcept {
    std::vector<UserTokenPolicy> result;
    std::string_view issuedTokenType{};
    std::string_view issuerEndpointUrl{};
    std::string_view securityPolicyUri{};
    if (allowAnonymous_) {
        result.emplace_back(
            policyIdAnonymous,
            UserTokenType::Anonymous,
            issuedTokenType,
            issuerEndpointUrl,
            securityPolicyUri
        );
    }
    if (!logins_.empty()) {
        result.emplace_back(
            policyIdUsername,
            UserTokenType::Username,
            issuedTokenType,
            issuerEndpointUrl,
            securityPolicyUri
        );
    }
    return result;
}

StatusCode AccessControlDefault::activateSession(
    [[maybe_unused]] Server& server,
    [[maybe_unused]] const NodeId& sessionId,
    [[maybe_unused]] const EndpointDescription& endpointDescription,
    [[maybe_unused]] const ByteString& secureChannelRemoteCertificate,
    const ExtensionObject& userIdentityToken
) noexcept {
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

void AccessControlDefault::closeSesion(
    [[maybe_unused]] Server& server, [[maybe_unused]] const NodeId& sessionId
) noexcept {}

uint32_t AccessControlDefault::getUserRightsMask(
    [[maybe_unused]] Server& server,
    [[maybe_unused]] const NodeId& sessionId,
    [[maybe_unused]] const NodeId& nodeId
) noexcept {
    return 0xFFFFFFFF;
}

uint8_t AccessControlDefault::getUserAccessLevel(
    [[maybe_unused]] Server& server,
    [[maybe_unused]] const NodeId& sessionId,
    [[maybe_unused]] const NodeId& nodeId
) noexcept {
    return 0xFF;
}

bool AccessControlDefault::getUserExecutable(
    [[maybe_unused]] Server& server,
    [[maybe_unused]] const NodeId& sessionId,
    [[maybe_unused]] const NodeId& methodId
) noexcept {
    return true;
}

bool AccessControlDefault::getUserExecutableOnObject(
    [[maybe_unused]] Server& server,
    [[maybe_unused]] const NodeId& sessionId,
    [[maybe_unused]] const NodeId& methodId,
    [[maybe_unused]] const NodeId& objectId
) noexcept {
    return true;
}

bool AccessControlDefault::allowAddNode(
    [[maybe_unused]] Server& server,
    [[maybe_unused]] const NodeId& sessionId,
    [[maybe_unused]] const AddNodesItem& item
) noexcept {
    return true;
}

bool AccessControlDefault::allowAddReference(
    [[maybe_unused]] Server& server,
    [[maybe_unused]] const NodeId& sessionId,
    [[maybe_unused]] const AddReferencesItem& item
) noexcept {
    return true;
}

bool AccessControlDefault::allowDeleteNode(
    [[maybe_unused]] Server& server,
    [[maybe_unused]] const NodeId& sessionId,
    [[maybe_unused]] const DeleteNodesItem& item
) noexcept {
    return true;
}

bool AccessControlDefault::allowDeleteReference(
    [[maybe_unused]] Server& server,
    [[maybe_unused]] const NodeId& sessionId,
    [[maybe_unused]] const DeleteReferencesItem& item
) noexcept {
    return true;
}

bool AccessControlDefault::allowBrowseNode(
    [[maybe_unused]] Server& server,
    [[maybe_unused]] const NodeId& sessionId,
    [[maybe_unused]] const NodeId& nodeId
) noexcept {
    return true;
}

#ifdef UA_ENABLE_SUBSCRIPTIONS
bool AccessControlDefault::allowTransferSubscription(
    [[maybe_unused]] Server& server,
    [[maybe_unused]] const NodeId& oldSessionId,
    [[maybe_unused]] const NodeId& newSessionId
) noexcept {
    return true;
}
#endif

#ifdef UA_ENABLE_HISTORIZING
bool AccessControlDefault::allowHistoryUpdate(
    [[maybe_unused]] Server& server,
    [[maybe_unused]] const NodeId& sessionId,
    [[maybe_unused]] const NodeId& nodeId,
    [[maybe_unused]] PerformUpdateType performInsertReplace,  // TODO
    [[maybe_unused]] const DataValue& value
) noexcept {
    return true;
}

bool AccessControlDefault::allowHistoryDelete(
    [[maybe_unused]] Server& server,
    [[maybe_unused]] const NodeId& sessionId,
    [[maybe_unused]] const NodeId& nodeId,
    [[maybe_unused]] DateTime startTimestamp,  // NOLINT
    [[maybe_unused]] DateTime endTimestamp,  // NOLINT
    [[maybe_unused]] bool isDeleteModified
) noexcept {
    return true;
}
#endif

}  // namespace opcua
