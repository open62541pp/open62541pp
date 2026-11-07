#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <utility>

#include <open62541pp/server.hpp>
#include <open62541pp/services/nodemanagement.hpp>

/// Templated data source that stores the data of type `T` internally.
template <typename T>
struct DataSource : public opcua::DataSourceBase {
    opcua::StatusCode read(
        [[maybe_unused]] opcua::Session& session,
        [[maybe_unused]] const opcua::NodeId& id,
        [[maybe_unused]] const opcua::NumericRange* range,
        opcua::DataValue& dv,
        bool timestamp
    ) override {
        std::cout << "Read value from data source: " << data << "\n";
        dv.setValue(opcua::Variant{data});
        if (timestamp) {
            dv.setSourceTimestamp(opcua::DateTime::now());
        }
        return UA_STATUSCODE_GOOD;
    }

    opcua::StatusCode write(
        [[maybe_unused]] opcua::Session& session,
        [[maybe_unused]] const opcua::NodeId& id,
        [[maybe_unused]] const opcua::NumericRange* range,
        const opcua::DataValue& dv
    ) override {
        data = dv.value().to<T>();
        std::cout << "Write value to data source: " << data << "\n";
        return UA_STATUSCODE_GOOD;
    }

    T data{};
};

#if UAPP_HAS_ASYNC_OPERATIONS && UAPP_OPEN62541_VER_GE(1, 5)
/// Async-capable data source (open62541 >= 1.5 only).
/// Reads alternate between answering right away and deferring the answer to a background thread,
/// writes are always deferred.
template <typename T>
struct AsyncDataSource : public opcua::DataSource<true, true> {
    opcua::StatusCode read(
        [[maybe_unused]] opcua::Session& session,
        [[maybe_unused]] const opcua::NodeId& id,
        [[maybe_unused]] const opcua::NumericRange* range,
        opcua::DataValue& dv,
        bool timestamp,
        opcua::AsyncReadContext& ctx
    ) override {
        const bool deferThisRead = deferNextRead;
        deferNextRead = !deferNextRead;
        if (deferThisRead) {
            auto [op, status] = ctx.defer();
            std::thread([operation = std::move(op), value = data.load()]() mutable {
                std::cout << "Reading value in a background thread...\n";
                std::this_thread::sleep_for(std::chrono::seconds{1});
                opcua::DataValue result;
                result.setValue(opcua::Variant{value});
                operation.complete(std::move(result));
                std::cout << "Read value from data source (async): " << value << "\n";
            }).detach();
            return status;
        }
        const auto value = data.load();
        std::cout << "Read value from data source (sync): " << value << "\n";
        dv.setValue(opcua::Variant{value});
        if (timestamp) {
            dv.setSourceTimestamp(opcua::DateTime::now());
        }
        return UA_STATUSCODE_GOOD;
    }

    opcua::StatusCode write(
        [[maybe_unused]] opcua::Session& session,
        [[maybe_unused]] const opcua::NodeId& id,
        [[maybe_unused]] const opcua::NumericRange* range,
        const opcua::DataValue& dv,
        opcua::AsyncWriteContext& ctx
    ) override {
        const auto value = dv.value().to<T>();
        auto [op, status] = ctx.defer();
        std::thread([this, operation = std::move(op), value]() mutable {
            std::cout << "Writing value in a background thread...\n";
            std::this_thread::sleep_for(std::chrono::seconds{1});
            data = value;
            operation.complete();
            std::cout << "Write value to data source (async): " << value << "\n";
        }).detach();
        return status;
    }

    // Atomic because it is accessed from both the server thread and the background threads
    std::atomic<T> data{};
    bool deferNextRead = true;
};
#endif

int main() {
    opcua::Server server;

    // Add variable node
    const auto id =
        opcua::services::addVariable(
            server,
            opcua::ObjectId::ObjectsFolder,
            {1, 1000},
            "DataSource",
            opcua::VariableAttributes{}
                .setAccessLevel(UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE)
                .setDataType<int>(),
            opcua::VariableTypeId::BaseDataVariableType,
            opcua::ReferenceTypeId::HasComponent
        )
            .value();

    // Define data source
    DataSource<int> dataSource;
    opcua::setVariableNodeValueBackend(server, id, dataSource);

#if UAPP_HAS_ASYNC_OPERATIONS && UAPP_OPEN62541_VER_GE(1, 5)
    // Add a second variable node backed by an async-capable data source.
    const auto asyncId =
        opcua::services::addVariable(
            server,
            opcua::ObjectId::ObjectsFolder,
            {1, 1001},
            "AsyncDataSource",
            opcua::VariableAttributes{}
                .setAccessLevel(UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE)
                .setDataType<int>(),
            opcua::VariableTypeId::BaseDataVariableType,
            opcua::ReferenceTypeId::HasComponent
        )
            .value();

    AsyncDataSource<int> asyncDataSource;
    opcua::setVariableNodeValueBackend(server, asyncId, asyncDataSource);
#endif

    server.run();
}
