#include <chrono>
#include <iostream>
#include <thread>

#include <open62541pp/server.hpp>
#include <open62541pp/services/nodemanagement.hpp>

int main() {
    opcua::Server server;

    const opcua::NodeId objectsId{opcua::ObjectId::ObjectsFolder};
    const opcua::NodeId methodId{1, 1000};

    const auto result = opcua::services::addMethod(
        server,
        objectsId,
        methodId,
        "Greet",
        [](opcua::Span<const opcua::Variant> input, opcua::Span<opcua::Variant> output) {
            const auto& name = input.at(0).scalar<opcua::String>();
            const auto greeting = std::string{"Hello "}.append(name);
            output.at(0) = greeting;
        },
        {{"name", {"en-US", "your name"}, opcua::DataTypeId::String, opcua::ValueRank::Scalar}},
        {{"greeting", {"en-US", "greeting"}, opcua::DataTypeId::String, opcua::ValueRank::Scalar}},
        opcua::MethodAttributes{},
        opcua::ReferenceTypeId::HasComponent
    );
    result.code().throwIfBad();

    // Enable async operation for the method node
    // This will queue the method call operations to be processed in a worker thread
    opcua::useAsyncOperation(server, methodId, true);

    // Process async operations in a separate worker thread
    std::thread workerThread{
        [&server]() {
            while (true) {
                const auto operation = opcua::getAsyncOperation(server);
                if (operation.has_value()) {
                    opcua::runAsyncOperation(server, operation.value());
                    std::cout << "Async operation processed\n";
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds{10});
                }
            }
        }
    };

    server.run();
    workerThread.join();
}
