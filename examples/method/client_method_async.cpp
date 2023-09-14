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
    auto clientThread = std::thread([&] { client.run(); });

    // Asynchronously call method
    auto future = opcua::services::callAsync(
        client,
        objNode.getNodeId(),
        greetMethodNode.getNodeId(),
        {opcua::Variant::fromScalar("World")}
    );

    std::cout << "Waiting for asynchronous operation to complete\n";
    future.wait();

    std::cout << "Future ready, get method output\n";
    auto output = future.get();
    std::cout << output.at(0).getScalar<opcua::String>() << std::endl;

    clientThread.join();
}
