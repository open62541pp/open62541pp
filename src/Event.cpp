#include "open62541pp/Event.h"

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"  // operator==
#include "open62541pp/overloads/comparison.h"  // operator==
#include "open62541pp/types/DateTime.h"
#include "open62541pp/types/Variant.h"

#include "open62541_impl.h"

namespace opcua {

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

Event::Event(Server& server, const NodeId& eventType)
    : connection_(server) {
    const auto status = UA_Server_createEvent(server.handle(), eventType, id_.handle());
    detail::throwOnBadStatus(status);
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
    return writeProperty({0, "SourceName"}, Variant::fromScalar(String(sourceName)));
}

Event& Event::writeTime(DateTime time) {
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
    detail::throwOnBadStatus(status);
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
    detail::throwOnBadStatus(status);
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
