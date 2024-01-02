// This example requires the server example `server_method` to be running.

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
    auto clientThread = std::thread([&] { client.run(0); });

    // Set up a callback with a promise to determine when to exit the example.
    std::promise<void> promise;
    auto future = promise.get_future();
    auto callback = [&promise](UA_StatusCode code, std::vector<opcua::Variant> output) {
        std::cout << "Response callback, status code: " << code << std::endl;
        std::cout << "Get method output\n";
        std::cout << output.at(0).getScalar<opcua::String>() << std::endl;
        promise.set_value();
    };

    // Asynchronously call method
    opcua::services::callAsync(
        client,
        objNode.getNodeId(),
        greetMethodNode.getNodeId(),
        {opcua::Variant::fromScalar("World")},
        callback
    );

    std::cout << "Waiting for asynchronous operation to complete\n";
    future.wait();

    client.stop();
    clientThread.join();
}
