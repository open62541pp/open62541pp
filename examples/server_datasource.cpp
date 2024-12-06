#include <iostream>

#include <open62541pp/server.hpp>
#include <open62541pp/services/nodemanagement.hpp>

int main() {
    opcua::Server server;

    // Counter variable as an example of a simple data source
    int counter = 0;

    // Add variable node
    const auto counterId = opcua::services::addVariable(
        server,
        opcua::ObjectId::ObjectsFolder,
        {1, 1000},
        "Counter",
        opcua::VariableAttributes{}
            .setAccessLevel(UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE)
            .setDataType<int>()
            .setValueScalar(counter),
        opcua::VariableTypeId::BaseDataVariableType,
        opcua::ReferenceTypeId::HasComponent
    ).value();

    // Define data source with its read and write callbacks
    opcua::ValueBackendDataSource dataSource;
    dataSource.read =
        [&](const opcua::NodeId&, opcua::DataValue& dv, const opcua::NumericRange&, bool timestamp) {
            // Increment counter before every read
            counter++;
            dv.value().setScalarCopy(counter);
            if (timestamp) {
                dv.setSourceTimestamp(opcua::DateTime::now());
            }
            std::cout << "Read counter from data source: " << counter << "\n";
            return UA_STATUSCODE_GOOD;
        };
    dataSource.write =
        [&](const opcua::NodeId&, const opcua::DataValue& dv, const opcua::NumericRange&) {
            counter = dv.value().getScalarCopy<int>();
            std::cout << "Write counter to data source: " << counter << "\n";
            return UA_STATUSCODE_GOOD;
        };

    // Define data source as variable node backend
    server.setVariableNodeValueBackend(counterId, dataSource);

    server.run();
}
