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

    // Asynchronously call method (callback variant)
    {
        opcua::services::callAsyncWithCallback(
            client,
            objNode.getNodeId(),
            greetMethodNode.getNodeId(),
            {opcua::Variant::fromScalar("Callback World")},
            [](UA_StatusCode code, std::vector<opcua::Variant> output) {
                std::cout << "Callback with status code " << code << ", get method output\n";
                std::cout << output.at(0).getScalar<opcua::String>() << std::endl;
            }
        );
    }

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

    client.stop();
    clientThread.join();
}
