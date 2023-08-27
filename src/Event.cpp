#include "open62541pp/Event.h"

#include "open62541pp/Config.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/Variant.h"

#include "open62541_impl.h"

namespace opcua {

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

Event::Event(Server& server, const NodeId& eventType)
    : server_(server) {
    const auto status = UA_Server_createEvent(server_.handle(), eventType, id_.handle());
    detail::throwOnBadStatus(status);
}

Event::~Event() {
    UA_Server_deleteNode(server_.handle(), id_, true /* deleteReferences */);  // ignore status
}

const NodeId& Event::getNodeId() const noexcept {
    return id_;
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
    const auto status = UA_Server_writeObjectProperty(server_.handle(), id_, propertyName, value);
    detail::throwOnBadStatus(status);
    return *this;
}

ByteString Event::trigger(const NodeId& originId) {
    ByteString eventId;
    const auto status = UA_Server_triggerEvent(
        server_.handle(), id_, originId, eventId.handle(), false /* deleteEventNode */
    );
    detail::throwOnBadStatus(status);
    return eventId;
}

#endif

}  // namespace opcua
