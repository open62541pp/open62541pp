#include "open62541pp/event.hpp"

#include "open62541pp/config.hpp"  // UA_ENABLE_SUBSCRIPTIONS_EVENTS
#include "open62541pp/detail/open62541/server.h"
#include "open62541pp/exception.hpp"
#include "open62541pp/server.hpp"
#include "open62541pp/types.hpp"

namespace opcua {

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

Event::Event(Server& connection, const NodeId& eventType)
    : connection_{&connection} {
    throwIfBad(UA_Server_createEvent(connection.handle(), eventType, id_.handle()));
}

Event::~Event() {
    UA_Server_deleteNode(connection().handle(), id(), true /* deleteReferences */);
}

Event& Event::writeSourceName(std::string_view sourceName) {
    return writeProperty({0, "SourceName"}, Variant{sourceName});
}

Event& Event::writeTime(DateTime time) {  // NOLINT(performance-unnecessary-value-param)
    return writeProperty({0, "Time"}, Variant{time});
}

Event& Event::writeSeverity(uint16_t severity) {
    return writeProperty({0, "Severity"}, Variant{severity});
}

Event& Event::writeMessage(const LocalizedText& message) {
    return writeProperty({0, "Message"}, Variant{message});
}

Event& Event::writeProperty(const QualifiedName& propertyName, const Variant& value) {
    const auto status = UA_Server_writeObjectProperty(
        connection().handle(), id(), propertyName, value
    );
    throwIfBad(status);
    return *this;
}

ByteString Event::trigger(const NodeId& originId) {
    ByteString eventId;
    const auto status = UA_Server_triggerEvent(
        connection().handle(),
        id(),
        originId,
        eventId.handle(),
        false  // deleteEventNode
    );
    throwIfBad(status);
    return eventId;
}

#endif

bool operator==(const Event& lhs, const Event& rhs) noexcept {
    return (lhs.connection() == rhs.connection()) && (lhs.id() == rhs.id());
}

bool operator!=(const Event& lhs, const Event& rhs) noexcept {
    return !(lhs == rhs);
}

}  // namespace opcua
