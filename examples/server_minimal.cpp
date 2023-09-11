#include "open62541pp/open62541pp.h"

int main() {
    opcua::Server server;

    // Add a variable node to the Objects node
    opcua::Node parentNode = server.getObjectsNode();
    opcua::Node myIntegerNode = parentNode.addVariable({1, 1000}, "TheAnswer");
    // Write some node attributes
    myIntegerNode.writeDisplayName({"en-US", "The Answer"})
        .writeDataType(opcua::DataTypeId::Int32)
        .writeValueScalar(42);

    server.run();
}
