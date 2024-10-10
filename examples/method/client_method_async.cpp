// This example requires the server example `server_method` to be running.

#include <iostream>
#include <thread>

#include <open62541pp/client.hpp>
#include <open62541pp/node.hpp>

int main() {
    opcua::Client client;
    client.connect("opc.tcp://localhost:4840");

    // Browse method node
    opcua::Node objectsNode(client, opcua::ObjectId::ObjectsFolder);
    const opcua::Node greetMethodNode = objectsNode.browseChild({{1, "Greet"}});

    // Run client event loop in separate thread
    auto clientThread = std::thread([&] { client.run(); });

    // Asynchronously call method (future variant - default)
    {
        auto future = opcua::services::callAsync(
            client,
            objectsNode.id(),
            greetMethodNode.id(),
            {opcua::Variant::fromScalar("Future World")},
            opcua::useFuture  // default, can be omitted
        );

        std::cout << "Waiting for asynchronous operation to complete\n";
        future.wait();

        std::cout << "Future ready, get method output\n";
        auto result = future.get();
        auto outputs = result.value().getOutputArguments();
        std::cout << outputs[0].getScalar<opcua::String>() << std::endl;
    }

    // Asynchronously call method (callback variant)
    {
        opcua::services::callAsync(
            client,
            objectsNode.id(),
            greetMethodNode.id(),
            {opcua::Variant::fromScalar("Callback World")},
            [](const opcua::Result<opcua::CallMethodResult>& result) {
                std::cout << "Callback invoked, get method output\n";
                auto outputs = result.value().getOutputArguments();
                std::cout << outputs[0].getScalar<opcua::String>() << std::endl;
            }
        );
    }

    client.stop();
    clientThread.join();
}
