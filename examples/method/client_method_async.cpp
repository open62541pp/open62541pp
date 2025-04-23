// This example requires the server example `server_method` to be running.

#include <iostream>
#include <thread>

#include <open62541pp/client.hpp>
#include <open62541pp/node.hpp>

int main() {
    opcua::Client client;
    client.connect("opc.tcp://localhost:4840");

    // Browse method node
    opcua::Node objectsNode{client, opcua::ObjectId::ObjectsFolder};
    const opcua::Node greetMethodNode = objectsNode.browseChild({{1, "Greet"}});

    // Run client event loop in separate thread
    auto clientThread = std::thread([&] { client.run(); });

    // Asynchronously call method (future variant - default)
    {
        auto future = objectsNode.callMethodAsync(
            greetMethodNode.id(), {opcua::Variant{"Future World"}}, opcua::useFuture
        );
        std::cout << "Waiting for asynchronous operation to complete\n";
        future.wait();

        std::cout << "Future ready, get method output\n";
        auto result = future.get();
        std::cout << "Future with status code: " << result.statusCode() << std::endl;
        std::cout << result.outputArguments()[0].scalar<opcua::String>() << std::endl;
    }

    // Asynchronously call method (callback variant)
    {
        objectsNode.callMethodAsync(
            greetMethodNode.id(),
            {opcua::Variant{"Callback World"}},
            [](opcua::CallMethodResult& result) {
                std::cout << "Callback with status code: " << result.statusCode() << std::endl;
                std::cout << result.outputArguments()[0].scalar<opcua::String>() << std::endl;
            }
        );
    }

    client.stop();
    clientThread.join();
}
