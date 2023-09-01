#include "open62541pp/Session.h"

#include <string>
#include <utility>

#include "open62541pp/Config.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

#include "open62541_impl.h"

namespace opcua {

Session::Session(Server& server, NodeId sessionId) noexcept
    : connection_(server),
      sessionId_(std::move(sessionId)) {}

Server& Session::getConnection() noexcept {
    return connection_;
}

const Server& Session::getConnection() const noexcept {
    return connection_;
}

const NodeId& Session::getSessionId() const noexcept {
    return sessionId_;
}

// ignore namespace index for v1.3, v1.4 uses qualified keys
[[maybe_unused]] inline static std::string unqualifiedKey(const QualifiedName& key) {
    return std::string{key.getName()};
}

Variant Session::getSessionAttribute([[maybe_unused]] const QualifiedName& key) {
    Variant variant;
#if UAPP_OPEN62541_VER_EQ(1, 3)
    const auto status = UA_Server_getSessionParameter(
        getConnection().handle(),
        getSessionId().handle(),
        unqualifiedKey(key).c_str(),
        variant.handle()
    );
    detail::throwOnBadStatus(status);
#endif
    return variant;
}

void Session::setSessionAttribute(
    [[maybe_unused]] const QualifiedName& key, [[maybe_unused]] const Variant& value
) {
#if UAPP_OPEN62541_VER_EQ(1, 3)
    const auto status = UA_Server_setSessionParameter(
        getConnection().handle(),
        getSessionId().handle(),
        unqualifiedKey(key).c_str(),
        value.handle()
    );
    detail::throwOnBadStatus(status);
#endif
}

void Session::deleteSessionAttribute([[maybe_unused]] const QualifiedName& key) {
#if UAPP_OPEN62541_VER_EQ(1, 3)
    UA_Server_deleteSessionParameter(
        getConnection().handle(), getSessionId().handle(), unqualifiedKey(key).c_str()
    );
#endif
}

void Session::close() {
#if UAPP_OPEN62541_VER_EQ(1, 3)
    const auto status = UA_Server_closeSession(getConnection().handle(), getSessionId().handle());
    detail::throwOnBadStatus(status);
#endif
}

bool operator==(const Session& lhs, const Session& rhs) noexcept {
    return (lhs.getConnection() == rhs.getConnection()) &&
           (lhs.getSessionId() == rhs.getSessionId());
}

bool operator!=(const Session& lhs, const Session& rhs) noexcept {
    return !(lhs == rhs);
}

}  // namespace opcua
