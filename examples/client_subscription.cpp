#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

#include <open62541pp/client.hpp>
#include <open62541pp/services/monitoreditem.hpp>
#include <open62541pp/services/subscription.hpp>

inline static std::atomic<bool> isRunning = true;  // NOLINT(*global-variables)

static void signalHandler(int sig) noexcept {
    if (sig == SIGINT || sig == SIGTERM) {
        isRunning = false;
    }
}

int main() {
    opcua::Client client;

    // Add a state callback (session activated) to create subscription(s) and monitored items(s) in
    // a newly activated session. If the client is disconnected, the subscriptions and monitored
    // items are deleted. This approach with the state callback assures, that the subscriptions are
    // recreated whenever the client reconnects to the server.
    client.onSessionActivated([&] {
        // Synchronous service calls are not allowed from a state callback because it is invoked
        // while the client's event loop is running. Use the asynchronous service API instead.
        opcua::SubscriptionParameters subscriptionParameters{};
        subscriptionParameters.publishingInterval = 1000.0;
        opcua::services::createSubscriptionAsync(
            client,
            subscriptionParameters,
            true,  // publishingEnabled
            {},  // statusChangeCallback
            {},  // deleteCallback
            [&](opcua::CreateSubscriptionResponse& response) {
                std::cout
                    << "Subscription created:\n"
                    << "- status code: " << response.responseHeader().serviceResult() << "\n"
                    << "- subscription id: " << response.subscriptionId() << std::endl;

                opcua::services::createMonitoredItemDataChangeAsync(
                    client,
                    response.subscriptionId(),
                    opcua::ReadValueId{
                        opcua::VariableId::Server_ServerStatus_CurrentTime,
                        opcua::AttributeId::Value
                    },
                    opcua::MonitoringMode::Reporting,
                    opcua::MonitoringParametersEx{},  // default monitoring parameters
                    [](opcua::IntegerId subId, opcua::IntegerId monId, const opcua::DataValue& dv) {
                        std::cout
                            << "Data change notification:\n"
                            << "- subscription id:   " << subId << "\n"
                            << "- monitored item id: " << monId << "\n"
                            << "- value:             " << opcua::toString(dv) << std::endl;
                    },
                    {},  // deleteCallback
                    [](opcua::MonitoredItemCreateResult& result) {
                        std::cout
                            << "Monitored item created:\n"
                            << "- status code: " << result.statusCode() << "\n"
                            << "- monitored item id: " << result.monitoredItemId() << std::endl;
                    }
                );
            }
        );
    });

    std::signal(SIGINT, signalHandler);  // NOLINT

    // Endless loop to automatically (try to) reconnect to server.
    while (isRunning) {
        try {
            client.connect("opc.tcp://localhost:4840");
            // Run the client's main loop to process callbacks and events.
            while (isRunning) {
                client.runIterate(100);
            }
        } catch (const opcua::BadStatus& e) {
            // Workaround to enforce a new session
            // https://github.com/open62541pp/open62541pp/issues/51
            client.disconnect();
            std::cout << "Error: " << e.what() << "\nRetry to connect in 3 seconds\n";
            std::this_thread::sleep_for(std::chrono::seconds{3});
        }
    }
}
