#include <iostream>

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
        ).value();

    // Define data source
    DataSource<int> dataSource;
    opcua::setVariableNodeValueBackend(server, id, dataSource);

    server.run();
}
