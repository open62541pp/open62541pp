#include <array>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <open62541pp/server.hpp>
#include <open62541pp/services/nodemanagement.hpp>

// Async method calls: before open62541 v1.5, the operations of the method node are queued and
// processed by a worker thread (useAsyncOperation, getAsyncOperation, runAsyncOperation).
// Since v1.5, the method callback defers its own completion with AsyncMethodContext::defer().

int main() {
    opcua::Server server;

    const opcua::NodeId objectsId{opcua::ObjectId::ObjectsFolder};
    const opcua::NodeId methodId{1, 1000};

#if UAPP_HAS_ASYNC_OPERATIONS && UAPP_OPEN62541_VER_GE(1, 5)
    const auto result = opcua::services::addMethod(
        server,
        objectsId,
        methodId,
        "Greet",
        [](opcua::Span<const opcua::Variant> input,
           [[maybe_unused]] opcua::Span<opcua::Variant> output,
           opcua::AsyncMethodContext& ctx) -> opcua::StatusCode {
            // Take what is needed from the input before deferring
            const auto& name = input.at(0).scalar<opcua::String>();
            const auto greeting = std::string{"Hello "}.append(name);
            // Complete the operation later from another thread
            auto [op, status] = ctx.defer();
            std::thread([operation = std::move(op), greeting]() mutable {
                std::this_thread::sleep_for(std::chrono::milliseconds{200});  // slow work
                std::array<opcua::Variant, 1> output2;
                output2[0] = greeting;
                operation.complete(output2);
                std::cout << "Async operation processed\n";
            }).detach();
            return status;
        },
        {{"name", {"en-US", "your name"}, opcua::DataTypeId::String, opcua::ValueRank::Scalar}},
        {{"greeting", {"en-US", "greeting"}, opcua::DataTypeId::String, opcua::ValueRank::Scalar}},
        opcua::MethodAttributes{},
        opcua::ReferenceTypeId::HasComponent
    );
    result.code().throwIfBad();

    server.run();
#else
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
    std::thread workerThread{[&server]() {
        while (true) {
            const auto operation = opcua::getAsyncOperation(server);
            if (operation.has_value()) {
                opcua::runAsyncOperation(server, operation.value());
                std::cout << "Async operation processed\n";
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds{10});
            }
        }
    }};

    server.run();
    workerThread.join();
#endif
}
