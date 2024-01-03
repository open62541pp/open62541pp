// This example requires the server example `server_method` to be running.

#include <exception>
#include <iostream>
#include <thread>

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Client client;
    client.connect("opc.tcp://localhost:4840");

    // Browse method node
    auto objNode = client.getObjectsNode();
    auto greetMethodNode = objNode.browseChild({{1, "Greet"}});

    // Run client event loop in separate thread
    auto clientThread = std::thread([&] { client.run(); });

    // Asynchronously call method (future variant)
    {
        auto future = opcua::services::callAsync(
            client,
            objNode.getNodeId(),
            greetMethodNode.getNodeId(),
            {opcua::Variant::fromScalar("Future World")}
        );

        std::cout << "Waiting for asynchronous operation to complete\n";
        future.wait();

        std::cout << "Future ready, get method output\n";
        auto output = future.get();
        std::cout << output.at(0).getScalar<opcua::String>() << std::endl;
    }

    // Asynchronously call method (callback variant)
    {
        // We use a promise similar to the future variant to determine when the callback is
        // executed.
        std::promise<std::vector<opcua::Variant>> promise;
        auto future = promise.get_future();
        auto callback = [&promise](UA_StatusCode code, std::vector<opcua::Variant> output) {
            if (code == UA_STATUSCODE_GOOD) {
                promise.set_value(std::move(output));
            } else {
                promise.set_exception(std::make_exception_ptr(opcua::BadStatus(code)));
            }
        };

        opcua::services::callAsync(
            client,
            objNode.getNodeId(),
            greetMethodNode.getNodeId(),
            {opcua::Variant::fromScalar("Callback World")},
            callback
        );

        std::cout << "Waiting for asynchronous operation to complete\n";
        future.wait();

        std::cout << "Future ready, get method output\n";
        auto output = future.get();
        std::cout << output.at(0).getScalar<opcua::String>() << std::endl;
    }

    client.stop();
    clientThread.join();
}
