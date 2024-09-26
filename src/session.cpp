#include "open62541pp/session.hpp"

#include <string>

#include "open62541pp/config.hpp"
#include "open62541pp/detail/open62541/server.h"
#include "open62541pp/exception.hpp"
#include "open62541pp/server.hpp"
#include "open62541pp/types.hpp"

namespace opcua {

// ignore namespace index for v1.3, v1.4 uses qualified keys
[[maybe_unused]] inline static std::string unqualify(const QualifiedName& key) {
    return std::string{key.getName()};
}

Variant Session::getSessionAttribute([[maybe_unused]] const QualifiedName& key) {
    Variant variant;
#if UAPP_OPEN62541_VER_EQ(1, 3)
    throwIfBad(UA_Server_getSessionParameter(
        connection().handle(), id().handle(), unqualify(key).c_str(), variant.handle()
    ));
#elif UAPP_OPEN62541_VER_GE(1, 4)
    throwIfBad(UA_Server_getSessionAttributeCopy(
        connection().handle(), id().handle(), key, variant.handle()
    ));
#endif
    return variant;
}

void Session::setSessionAttribute(
    [[maybe_unused]] const QualifiedName& key, [[maybe_unused]] const Variant& value
) {
#if UAPP_OPEN62541_VER_EQ(1, 3)
    throwIfBad(UA_Server_setSessionParameter(
        connection().handle(), id().handle(), unqualify(key).c_str(), value.handle()
    ));
#elif UAPP_OPEN62541_VER_GE(1, 4)
    throwIfBad(
        UA_Server_setSessionAttribute(connection().handle(), id().handle(), key, value.handle())
    );
#endif
}

void Session::deleteSessionAttribute([[maybe_unused]] const QualifiedName& key) {
#if UAPP_OPEN62541_VER_EQ(1, 3)
    UA_Server_deleteSessionParameter(connection().handle(), id().handle(), unqualify(key).c_str());
#elif UAPP_OPEN62541_VER_GE(1, 4)
    throwIfBad(UA_Server_deleteSessionAttribute(connection().handle(), id().handle(), key));
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
