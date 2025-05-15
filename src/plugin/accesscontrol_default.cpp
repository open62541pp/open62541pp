#include "open62541pp/plugin/accesscontrol_default.hpp"

#include <cmath>
#include <string_view>
#include <utility>  // move

#include "open62541pp/detail/open62541/common.h"  // UA_STATUSCODE_*

namespace opcua {

constexpr std::string_view policyIdAnonymous = "open62541-anonymous-policy";
constexpr std::string_view policyIdUsername = "open62541-username-policy";

static constexpr bool startsWith(std::string_view str, std::string_view prefix) noexcept {
    const auto common = std::min(str.size(), prefix.size());
    return str.substr(0, common) == prefix.substr(0, common);
}

AccessControlDefault::AccessControlDefault(bool allowAnonymous, Span<const Login> logins)
    : allowAnonymous_{allowAnonymous},
      logins_{logins.begin(), logins.end()} {
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
    if (userIdentityToken.empty()) {
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
    if (const auto* token = userIdentityToken.decodedData<AnonymousIdentityToken>();
        token != nullptr) {
        if (!allowAnonymous_) {
            return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
        }
        if (token->policyId().empty() || startsWith(token->policyId(), policyIdAnonymous)) {
            return UA_STATUSCODE_GOOD;
        }
        return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
    }

    // username and password
    if (const auto* token = userIdentityToken.decodedData<UserNameIdentityToken>();
        token != nullptr) {
        if (!startsWith(token->policyId(), policyIdUsername)) {
            return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
        }
        // empty username and password
        if (token->userName().empty() && token->password().empty()) {
            return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
        }
        // try to match username / password
        for (const auto& login : logins_) {
            if ((login.username == token->userName()) && (login.password == token->password())) {
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
    [[maybe_unused]] DateTime startTimestamp,
    [[maybe_unused]] DateTime endTimestamp,
    [[maybe_unused]] bool isDeleteModified
) {
    return true;
}

}  // namespace opcua
