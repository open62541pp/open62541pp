// This example should be executed while `server_events` is running. You can use e.g. UaExpert to
// trigger events with custom severity and messages using the `GenerateEvent` method node.

#include <iostream>

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Client client;
    client.connect("opc.tcp://localhost:4840");

    // Define filter elements
    const opcua::ContentFilterElement filterBaseEventType(
        opcua::FilterOperator::OfType,
        {
            opcua::LiteralOperand(opcua::NodeId(opcua::ObjectTypeId::BaseEventType)),
        }
    );
    const opcua::ContentFilterElement filterLowSeverity(
        opcua::FilterOperator::LessThan,
        {
            opcua::SimpleAttributeOperand(
                opcua::ObjectTypeId::BaseEventType, {{0, "Severity"}}, opcua::AttributeId::Value
            ),
            opcua::LiteralOperand(200),
        }
    );

    // Combine filter with unary operators (!) and binary operators (&&, ||).
    // The resulting filter will filter events of type BaseEventType with severity >= 200.
    const opcua::ContentFilter filterCombined = filterBaseEventType && !filterLowSeverity;

    const opcua::EventFilter eventFilter(
        // Select clause
        {
            {opcua::ObjectTypeId::BaseEventType, {{0, "Time"}}, opcua::AttributeId::Value},
            {opcua::ObjectTypeId::BaseEventType, {{0, "Severity"}}, opcua::AttributeId::Value},
            {opcua::ObjectTypeId::BaseEventType, {{0, "Message"}}, opcua::AttributeId::Value},
        },
        // Where clause
        filterCombined
    );

    auto sub = client.createSubscription();
    sub.subscribeEvent(
        opcua::ObjectId::Server,
        eventFilter,
        [&](const auto& item, opcua::Span<const opcua::Variant> eventFields) {
            std::cout
                << "Event notification:\n"
                << "- subscription id:   " << item.getSubscriptionId() << "\n"
                << "- monitored item id: " << item.getMonitoredItemId() << "\n"
                << "- node id:           " << item.getNodeId().toString() << "\n"
                << "- attribute id:      " << static_cast<int>(item.getAttributeId()) << "\n";

            const auto& time = eventFields[0].getScalar<opcua::DateTime>();
            const auto& severity = eventFields[1].getScalar<uint16_t>();
            const auto& message = eventFields[2].getScalar<opcua::LocalizedText>();

            std::cout << "Time:     " << time.format("%Y-%m-%d %H:%M:%S") << "\n";
            std::cout << "Severity: " << severity << "\n";
            std::cout << "Message:  " << message.getText() << "\n";
        }
    );

    // Run the client's main loop to process callbacks and events.
    // This will block until client.stop() is called or an exception is thrown.
    client.run();
}
