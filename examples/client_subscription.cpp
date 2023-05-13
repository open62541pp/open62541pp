#include <chrono>
#include <iostream>
#include <thread>

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Client client;
    client.connect("opc.tcp://localhost:4840");

    auto sub = client.createSubscription();

    // Modify and delete the subscription via the returend Subscription<T> object
    opcua::SubscriptionParameters subscriptionParameters{};
    subscriptionParameters.publishingInterval = 1000.0;
    sub.setSubscriptionParameters(subscriptionParameters);
    sub.setPublishingMode(true);
    // sub.deleteSubscription();

    // Create a monitored item within the subscription for data change notifications
    auto mon = sub.subscribeDataChange(
        opcua::VariableId::Server_ServerStatus_CurrentTime,  // monitored node id
        opcua::AttributeId::Value,  // monitored attribute
        [](const auto& item, const opcua::DataValue& value) {
            std::cout << "Data change notification:\n"
                      << "- subscription id:   " << item.getSubscriptionId() << "\n"
                      << "- monitored item id: " << item.getMonitoredItemId() << "\n"
                      << "- node id:           " << item.getNodeId().toString() << "\n"
                      << "- attribute id:      " << static_cast<int>(item.getAttributeId()) << "\n";

            const auto dt = value.getValue().getScalarCopy<opcua::DateTime>();
            std::cout << "Current server time (UTC): " << dt.format("%H:%M:%S") << std::endl;
        }
    );

    // Modify and delete the monitored item via the returned MonitoredItem<T> object
    opcua::MonitoringParameters monitoringParameters{};
    monitoringParameters.samplingInterval = 100.0;
    mon.setMonitoringParameters(monitoringParameters);
    mon.setMonitoringMode(opcua::MonitoringMode::Reporting);
    // mon.deleteMonitoredItem();

    // Run the client's main loop to process callbacks and events.
    // This will block until client.stop() is called or an exception is thrown.
    client.run();
}
