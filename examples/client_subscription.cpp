#include <chrono>
#include <iostream>
#include <thread>

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Client client;
    client.connect("opc.tcp://localhost:4840");

    opcua::SubscriptionParameters subscriptionParameters{};
    subscriptionParameters.publishingInterval = 1000.0;

    auto sub = client.createSubscription(subscriptionParameters);

    sub.subscribeDataChange(
        opcua::VariableId::Server_ServerStatus_CurrentTime,
        opcua::AttributeId::Value,
        [](const auto& item, const opcua::DataValue& value) {
            std::cout << "Data change notification:\n"
                      << "- subscription id:   " << item.getSubscriptionId() << "\n"
                      << "- monitored item id: " << item.getMonitoredItemId() << "\n";

            const auto dt = value.getValue().value().getScalarCopy<opcua::DateTime>();
            std::cout << "Current server time (UTC): " << dt.format("%H:%M:%S") << std::endl;
        }
    );

    while (true) {
        client.runIterate();
    }
}
