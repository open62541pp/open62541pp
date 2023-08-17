#include <iostream>

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Server server(4840);

    // Counter variable as an example of a simple data source
    int counter = 0;

    // Add variable node
    auto nodeCounter = server.getObjectsNode().addVariable(
        {1, 1000},
        "Counter",
        opcua::VariableAttributes{}
            .setAccessLevel(UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE)
            .setDataType<int>()
            .setValueScalar(counter)
    );

    // Define data source with its read and write callbacks
    opcua::ValueBackendDataSource dataSource;
    dataSource.read = [&](opcua::DataValue& value, const opcua::NumericRange&, bool timestamp) {
        // Increment counter before every read
        counter++;
        value.getValue().setScalar(counter);
        if (timestamp) {
            value.setSourceTimestamp(opcua::DateTime::now());
        }
        std::cout << "Read counter from data source: " << counter << "\n";
        return UA_STATUSCODE_GOOD;
    };
    dataSource.write = [&](const opcua::DataValue& value, const opcua::NumericRange&) {
        counter = value.getValue().getScalarCopy<int>();
        std::cout << "Write counter to data source: " << counter << "\n";
        return UA_STATUSCODE_GOOD;
    };

    // Define data source as variable node backend
    server.setVariableNodeValueBackend(nodeCounter.getNodeId(), dataSource);

    server.run();
}
