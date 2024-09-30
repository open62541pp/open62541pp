#include <open62541pp/node.hpp>
#include <open62541pp/server.hpp>

int main() {
    opcua::Server server;

    // Add a variable node to the Objects node
    opcua::Node parentNode(server, opcua::ObjectId::ObjectsFolder);
    opcua::Node myIntegerNode = parentNode.addVariable({1, 1000}, "TheAnswer");
    // Write value attribute
    myIntegerNode.writeValueScalar(42);

    server.run();
}
