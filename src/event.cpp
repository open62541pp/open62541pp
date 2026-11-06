#include "open62541pp/event.hpp"

#include <string>
#include <utility>  // move

#include "open62541pp/config.hpp"  // UA_ENABLE_SUBSCRIPTIONS_EVENTS
#include "open62541pp/exception.hpp"
#include "open62541pp/server.hpp"
#include "open62541pp/types.hpp"

namespace opcua {

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#if UAPP_OPEN62541_VER_GE(1, 5)
Event::Event(Server& connection, NodeId eventType)
    : connection_{&connection},
      eventType_{std::move(eventType)} {}
#else

Event::Event(Server& connection, const NodeId& eventType)
    : connection_{&connection} {
    throwIfBad(UA_Server_createEvent(connection.handle(), eventType, id_.handle()));
}
#endif

Event::~Event()
#if UAPP_OPEN62541_VER_GE(1, 5)
    = default;
#else
{
    if (!id().isNull()) {
        UA_Server_deleteNode(connection().handle(), id(), true /* deleteReferences */);
    }
}
#endif

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
#if UAPP_OPEN62541_VER_GE(1, 5)
    // event fields are addressed by their browse path, the namespace index of the key is the
    // default namespace index of the path elements
    const std::string_view name = propertyName.name();
    const bool isPath = !name.empty() && name.front() == '/';
    props_.set(
        isPath ? propertyName
               : QualifiedName(propertyName.namespaceIndex(), "/" + std::string(name)),
        value
    );
#else
    const auto status = UA_Server_writeObjectProperty(
        connection().handle(), id(), propertyName, value
    );
    throwIfBad(status);
#endif
    return *this;
}

ByteString Event::trigger(const NodeId& originId) {
    ByteString eventId;

#if UAPP_OPEN62541_VER_GE(1, 5)
    const UA_UInt16 severity = 1;
    LocalizedText message{"en-US", "Event"};

    if (!props_.contains(QualifiedName(0, "/Message")) && !originId.isNull()) {
        QualifiedName browseName;
        throwIfBad(UA_Server_readBrowseName(connection_->handle(), originId, browseName.handle()));

        message = LocalizedText{"", browseName.name()};
    }

    const auto status = UA_Server_createEvent(
        connection_->handle(),
        originId,
        eventType_,
        severity,
        *message.handle(),
        props_.handle(),
        nullptr,
        eventId.handle()
    );
#else
    const auto status = UA_Server_triggerEvent(
        connection().handle(),
        id(),
        originId,
        eventId.handle(),
        false  // deleteEventNode
    );
#endif

    throwIfBad(status);
    return eventId;
}

#if UAPP_OPEN62541_VER_LE(1, 4)

NodeId Event::release() noexcept {
    return std::exchange(id_, {});
}

bool operator==(const Event& lhs, const Event& rhs) noexcept {
    return (lhs.connection() == rhs.connection()) && (lhs.id() == rhs.id());
}

bool operator!=(const Event& lhs, const Event& rhs) noexcept {
    return !(lhs == rhs);
}

#endif

#endif

}  // namespace opcua
