#include "open62541pp/open62541pp.h"

int main() {
    opcua::Server server;

    // Add a variable node to the Objects node
    auto parentNode = server.getObjectsNode();
    auto myIntegerNode = parentNode.addVariable({1, "the.answer"}, "the answer");
    // Write some node attributes
    myIntegerNode.writeDataType(opcua::Type::Int32)
        .writeDisplayName({"en-US", "the answer"})
        .writeDescription({"en-US", "the answer"})
        .writeScalar(42);

    server.run();
}
