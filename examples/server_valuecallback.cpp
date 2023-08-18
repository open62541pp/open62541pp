#include <iostream>

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Server server;

    const opcua::NodeId currentTimeId(1, "CurrentTime");
    auto currentTimeNode =
        server.getObjectsNode()
            .addVariable(currentTimeId, "CurrentTime")
            .writeDisplayName({"en-US", "Current time"})
            .writeDescription({"en-US", "Current time"})
            .writeDataType<opcua::DateTime>()
            .writeValueScalar(opcua::DateTime::now());

    // set variable value callback to write current time before every read operation
    opcua::ValueCallback valueCallback;
    valueCallback.onBeforeRead = [&](const opcua::DataValue& value) {
        const auto timeOld = value.getValue().getScalar<opcua::DateTime>();
        const auto timeNow = opcua::DateTime::now();
        std::cout << "Time before read: " << timeOld.format("%Y-%m-%d %H:%M:%S") << std::endl;
        std::cout << "Set current time: " << timeNow.format("%Y-%m-%d %H:%M:%S") << std::endl;
        currentTimeNode.writeValueScalar(timeNow);
    };
    server.setVariableNodeValueCallback(currentTimeId, valueCallback);

    server.run();
}
