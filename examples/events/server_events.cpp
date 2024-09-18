#include "open62541pp/open62541pp.h"

int main() {
    opcua::Server server;

    // Objects node can be used to subscribe to events.
    opcua::Node objectsNode(server, opcua::ObjectId::ObjectsFolder);
    objectsNode.writeEventNotifier(opcua::EventNotifier::SubscribeToEvents);

    // Add a method to generate and trigger events.
    opcua::Event event(server);
    objectsNode.addMethod(
        {1, 1000},
        "GenerateEvent",
        [&](opcua::Span<const opcua::Variant> input, opcua::Span<opcua::Variant>) {
            const auto severity = input[0].getScalar<uint16_t>();
            const auto& message = input[1].getScalar<opcua::String>();

            event.writeTime(opcua::DateTime::now());
            event.writeSeverity(severity);
            event.writeMessage({"", message});
            event.trigger();
        },
        {
            {"severity", {"", "Severity"}, opcua::DataTypeId::UInt16, opcua::ValueRank::Scalar},
            {"message", {"", "Message"}, opcua::DataTypeId::String, opcua::ValueRank::Scalar},
        },
        {}
    );

    server.run();
}
