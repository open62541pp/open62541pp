#include "open62541pp/Event.h"

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"  // operator==
#include "open62541pp/detail/open62541/config.h"
#include "open62541pp/detail/open62541/server.h"
#include "open62541pp/types/DateTime.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

namespace opcua {

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

Event::Event(Server& server, const NodeId& eventType)
    : connection_(server) {
    const auto status = UA_Server_createEvent(server.handle(), eventType, id_.handle());
    throwIfBad(status);
}

Event::~Event() {
    UA_Server_deleteNode(getConnection().handle(), getNodeId(), true /* deleteReferences */);
}

Server& Event::getConnection() noexcept {
    return connection_;
}

const Server& Event::getConnection() const noexcept {
    return connection_;
}

const NodeId& Event::getNodeId() const noexcept {
    return id_;
}

Event& Event::writeSourceName(std::string_view sourceName) {
    return writeProperty({0, "SourceName"}, Variant::fromScalar(sourceName));
}

Event& Event::writeTime(DateTime time) {  // NOLINT
    return writeProperty({0, "Time"}, Variant::fromScalar(time));
}

Event& Event::writeSeverity(uint16_t severity) {
    return writeProperty({0, "Severity"}, Variant::fromScalar(severity));
}

Event& Event::writeMessage(const LocalizedText& message) {
    return writeProperty({0, "Message"}, Variant::fromScalar(message));
}

Event& Event::writeProperty(const QualifiedName& propertyName, const Variant& value) {
    const auto status = UA_Server_writeObjectProperty(
        getConnection().handle(), getNodeId(), propertyName, value
    );
    throwIfBad(status);
    return *this;
}

ByteString Event::trigger(const NodeId& originId) {
    ByteString eventId;
    const auto status = UA_Server_triggerEvent(
        getConnection().handle(),
        getNodeId(),
        originId,
        eventId.handle(),
        false  // deleteEventNode
    );
    throwIfBad(status);
    return eventId;
}

#endif

bool operator==(const Event& lhs, const Event& rhs) noexcept {
    return (lhs.getConnection() == rhs.getConnection()) && (lhs.getNodeId() == rhs.getNodeId());
}

bool operator!=(const Event& lhs, const Event& rhs) noexcept {
    return !(lhs == rhs);
}

}  // namespace opcua
