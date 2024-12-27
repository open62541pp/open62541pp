#include <iostream>

#include <open62541pp/node.hpp>
#include <open62541pp/server.hpp>

// Variable value callback to write current time before every read operation
class CurrentTimeCallback : public opcua::ValueCallbackBase {
    void onRead(
        opcua::Session& session,
        const opcua::NodeId& id,
        [[maybe_unused]] const opcua::NumericRange* range,
        const opcua::DataValue& value
    ) override {
        opcua::Node node(session.connection(), id);
        const auto timeOld = value.value().scalar<opcua::DateTime>();
        const auto timeNow = opcua::DateTime::now();
        std::cout << "Time before read: " << timeOld.format("%Y-%m-%d %H:%M:%S") << std::endl;
        std::cout << "Set current time: " << timeNow.format("%Y-%m-%d %H:%M:%S") << std::endl;
        node.writeValueScalar(timeNow);
    }

    void onWrite(
        [[maybe_unused]] opcua::Session& session,
        [[maybe_unused]] const opcua::NodeId& id,
        [[maybe_unused]] const opcua::NumericRange* range,
        [[maybe_unused]] const opcua::DataValue& value
    ) override {}
};

int main() {
    opcua::Server server;

    const opcua::NodeId currentTimeId(1, "CurrentTime");
    opcua::Node(server, opcua::ObjectId::ObjectsFolder)
        .addVariable(currentTimeId, "CurrentTime")
        .writeDisplayName({"en-US", "Current time"})
        .writeDescription({"en-US", "Current time"})
        .writeDataType<opcua::DateTime>()
        .writeValueScalar(opcua::DateTime::now());

    CurrentTimeCallback currentTimeCallback;
    server.setVariableNodeValueCallback(currentTimeId, currentTimeCallback);

    server.run();
}
