#include <iostream>

#include <open62541pp/client.hpp>
#include <open62541pp/services/attribute_highlevel.hpp>
#include <open62541pp/services/monitoreditem.hpp>
#include <open62541pp/services/subscription.hpp>
#include <open62541pp/services/view.hpp>

int main() {
    opcua::Client client;

    client.onConnected([] { std::cout << "Client connected\n"; });
    client.onSessionActivated([&] {
        std::cout << "Session activated\n";

        // Schedule async operations once the client session is activated
        // 1. Read request
        opcua::services::readValueAsync(
            client,
            opcua::VariableId::Server_ServerStatus_CurrentTime,
            [](opcua::Result<opcua::Variant>& result) {
                std::cout << "Read result with status code: " << result.code() << "\n";
                const auto dt = result.value().getScalar<opcua::DateTime>();
                std::cout << "Server date (UTC): " << dt.format("%Y-%m-%d %H:%M:%S") << "\n";
            }
        );

        // 2. Browse request of Server object
        const opcua::BrowseDescription bd(
            opcua::ObjectId::Server,
            opcua::BrowseDirection::Forward,
            opcua::ReferenceTypeId::References
        );
        opcua::services::browseAsync(client, bd, 0, [](opcua::BrowseResult& result) {
            std::cout << "Browse result with " << result.getReferences().size() << " references:\n";
            for (const auto& reference : result.getReferences()) {
                std::cout << reference.getBrowseName().getName() << "\n";
            }
        });

        // 3. Subscription
        opcua::services::createSubscriptionAsync(
            client,
            opcua::SubscriptionParameters{},  // default subscription parameters
            true,  // publishingEnabled
            {},  // statusChangeCallback
            [](uint32_t subId) { std::cout << "Subscription deleted: " << subId << "\n"; },
            [&](opcua::CreateSubscriptionResponse& response) {
                std::cout
                    << "Subscription created\n"
                    << "- status code: " << response.getResponseHeader().getServiceResult() << "\n"
                    << "- subscription id: " << response.getSubscriptionId() << "\n";

                // Create MonitoredItem
                opcua::services::createMonitoredItemDataChangeAsync(
                    client,
                    response.getSubscriptionId(),
                    opcua::ReadValueId(
                        opcua::VariableId::Server_ServerStatus_CurrentTime,
                        opcua::AttributeId::Value
                    ),
                    opcua::MonitoringMode::Reporting,
                    opcua::MonitoringParametersEx{},  // default monitoring parameters
                    [](uint32_t subId, uint32_t monId, const opcua::DataValue& dv) {
                        std::cout
                            << "Data change notification:\n"
                            << "- subscription id: " << subId << "\n"
                            << "- monitored item id: " << monId << "\n"
                            << "- timestamp: " << dv.getSourceTimestamp() << "\n";
                    },
                    {},  // delete callback
                    [](opcua::MonitoredItemCreateResult& result) {
                        std::cout
                            << "MonitoredItem created\n"
                            << "- status code: " << result.getStatusCode() << "\n"
                            << "- monitored item id: " << result.getMonitoredItemId() << "\n";
                    }
                );
            }
        );
    });
    client.onSessionClosed([] { std::cout << "Session closed\n"; });
    client.onDisconnected([] { std::cout << "Client disconnected\n"; });

    client.connectAsync("opc.tcp://localhost:4840");
    client.run();
}
