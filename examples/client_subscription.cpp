#include <chrono>
#include <iostream>
#include <thread>

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Client client;
    client.connect("opc.tcp://localhost:4840");

    auto sub = client.createSubscription();

    // modify and delete the subscription via the returend Subscription<T> object
    opcua::SubscriptionParameters subscriptionParameters{};
    subscriptionParameters.publishingInterval = 1000.0;
    sub.setSubscriptionParameters(subscriptionParameters);
    sub.setPublishingMode(true);
    // sub.deleteSubscription();

    // create a monitored item within the subscription for data change notifications
    auto mon = sub.subscribeDataChange(
        opcua::VariableId::Server_ServerStatus_CurrentTime,  // monitored node id
        opcua::AttributeId::Value,  // monitored attribute
        [](const auto& item, const opcua::DataValue& value) {
            std::cout << "Data change notification:\n"
                      << "- subscription id:   " << item.getSubscriptionId() << "\n"
                      << "- monitored item id: " << item.getMonitoredItemId() << "\n";

            const auto dt = value.getValue().value().getScalarCopy<opcua::DateTime>();
            std::cout << "Current server time (UTC): " << dt.format("%H:%M:%S") << std::endl;
        }
    );

    // modify and delete the monitored item via the returned MonitoredItem<T> object
    opcua::MonitoringParameters monitoringParameters{};
    monitoringParameters.samplingInterval = 100.0;
    mon.setMonitoringParameters(monitoringParameters);
    mon.setMonitoringMode(opcua::MonitoringMode::Reporting);
    // mon.deleteMonitoredItem();

    while (true) {
        client.runIterate();
    }
}
