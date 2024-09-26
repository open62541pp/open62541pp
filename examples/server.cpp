#include <iostream>

#include "open62541pp/open62541pp.hpp"

int main() {
    opcua::Server server(4840 /* port */);

    server.setApplicationName("open62541pp server example");
    server.setApplicationUri("urn:open62541pp.server.application");
    server.setProductUri("https://open62541pp.github.io");

    // Add a variable node to the Objects node
    opcua::Node parentNode(server, opcua::ObjectId::ObjectsFolder);
    opcua::Node myIntegerNode = parentNode.addVariable(
        {1, "TheAnswer"},
        "The Answer",
        opcua::VariableAttributes{}
            .setDisplayName({"en-US", "The Answer"})
            .setDescription({"en-US", "Answer to the Ultimate Question of Life"})
            .setDataType<int>()
    );

    // Write a value (attribute) to the node
    myIntegerNode.writeValueScalar(42);

    // Read the value (attribute) from the node
    std::cout << "The answer is: " << myIntegerNode.readValueScalar<int>() << std::endl;

    server.run();
}
