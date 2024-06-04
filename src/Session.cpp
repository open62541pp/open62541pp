#include "open62541pp/Session.h"

#include <string>

#include "open62541pp/Config.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/open62541/server.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

namespace opcua {

// ignore namespace index for v1.3, v1.4 uses qualified keys
[[maybe_unused]] inline static std::string unqualifiedKey(const QualifiedName& key) {
    return std::string{key.getName()};
}

Variant Session::getSessionAttribute([[maybe_unused]] const QualifiedName& key) {
    Variant variant;
#if UAPP_OPEN62541_VER_GE(1, 4)
    const auto status = UA_Server_getSessionAttribute(
        connection().handle(), id().handle(), key, variant.handle()
    );
    throwIfBad(status);
#elif UAPP_OPEN62541_VER_GE(1, 3)
    const auto status = UA_Server_getSessionParameter(
        connection().handle(), id().handle(), unqualifiedKey(key).c_str(), variant.handle()
    );
    throwIfBad(status);
#endif
    return variant;
}

void Session::setSessionAttribute(
    [[maybe_unused]] const QualifiedName& key, [[maybe_unused]] const Variant& value
) {
#if UAPP_OPEN62541_VER_GE(1, 4)
    const auto status = UA_Server_setSessionAttribute(
        connection().handle(), id().handle(), key, value.handle()
    );
    throwIfBad(status);
#elif UAPP_OPEN62541_VER_GE(1, 3)
    const auto status = UA_Server_setSessionParameter(
        connection().handle(), id().handle(), unqualifiedKey(key).c_str(), value.handle()
    );
    throwIfBad(status);
#endif
}

void Session::deleteSessionAttribute([[maybe_unused]] const QualifiedName& key) {
#if UAPP_OPEN62541_VER_GE(1, 4)
    const auto status = UA_Server_deleteSessionAttribute(connection().handle(), id().handle(), key);
    throwIfBad(status);
#elif UAPP_OPEN62541_VER_GE(1, 3)
    UA_Server_deleteSessionParameter(
        connection().handle(), id().handle(), unqualifiedKey(key).c_str()
    );
#endif
}

void Session::close() {
#if UAPP_OPEN62541_VER_GE(1, 3)
    throwIfBad(UA_Server_closeSession(connection().handle(), id().handle()));
#endif
}

bool operator==(const Session& lhs, const Session& rhs) noexcept {
    return (lhs.connection() == rhs.connection()) && (lhs.id() == rhs.id());
}

bool operator!=(const Session& lhs, const Session& rhs) noexcept {
    return !(lhs == rhs);
}

}  // namespace opcua
