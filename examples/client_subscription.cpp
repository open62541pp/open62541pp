#include <chrono>
#include <iostream>
#include <thread>

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Client client;

    // Add a state callback (session activated) to create subscription(s) and monitored items(s) in
    // a newly activated session. If the client is disconnected, the subscriptions and monitored
    // items are deleted. This approach with the state callback assures, that the subscriptions are
    // recreated whenever the client reconnects to the server.
    client.onSessionActivated([&] {
        auto sub = client.createSubscription();

        // Modify and delete the subscription via the returned Subscription<T> object
        opcua::SubscriptionParameters subscriptionParameters{};
        subscriptionParameters.publishingInterval = 1000.0;
        sub.setSubscriptionParameters(subscriptionParameters);
        sub.setPublishingMode(true);
        // sub.deleteSubscription();

        // Create a monitored item within the subscription for data change notifications
        auto mon = sub.subscribeDataChange(
            opcua::VariableId::Server_ServerStatus_CurrentTime,  // monitored node id
            opcua::AttributeId::Value,  // monitored attribute
            [&](const auto& item, const opcua::DataValue& value) {
                std::cout
                    << "Data change notification:\n"
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
    });

    // Endless loop to automatically (try to) reconnect to server.
    while (true) {
        try {
            client.connect("opc.tcp://localhost:4840");
            // Run the client's main loop to process callbacks and events.
            // This will block until client.stop() is called or an exception is thrown.
            client.run();
        } catch (const opcua::BadDisconnect&) {
            // Workaround to enforce a new session
            // https://github.com/open62541pp/open62541pp/issues/51
            client.disconnect();
            std::cout << "Disconnected. Retry to connect in 3 seconds\n";
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }
}
