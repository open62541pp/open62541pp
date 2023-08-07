#include "open62541pp/Session.h"

#include <string>
#include <utility>

#include "open62541pp/Config.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/types/Builtin.h"
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

#if UAPP_OPEN62541_VER_GE(1, 3)
// ignore namespace index for v1.3, v1.4 uses qualified keys
inline static std::string unqualifiedKey(const QualifiedName& key) {
    return std::string{key.getName()};
}

Variant Session::getSessionAttribute(const QualifiedName& key) {
    Variant variant;
    const auto status = UA_Server_getSessionParameter(
        getConnection().handle(),
        getSessionId().handle(),
        unqualifiedKey(key).c_str(),
        variant.handle()
    );
    detail::throwOnBadStatus(status);
    return variant;
}

void Session::setSessionAttribute(const QualifiedName& key, const Variant& value) {
    const auto status = UA_Server_setSessionParameter(
        getConnection().handle(),
        getSessionId().handle(),
        unqualifiedKey(key).c_str(),
        value.handle()
    );
    detail::throwOnBadStatus(status);
}

void Session::deleteSessionAttribute(const QualifiedName& key) {
    UA_Server_deleteSessionParameter(
        getConnection().handle(), getSessionId().handle(), unqualifiedKey(key).c_str()
    );
}

void Session::close() {
    const auto status = UA_Server_closeSession(getConnection().handle(), getSessionId().handle());
    detail::throwOnBadStatus(status);
}
#endif

}  // namespace opcua
