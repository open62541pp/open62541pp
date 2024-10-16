#include <iostream>

#include <open62541pp/node.hpp>
#include <open62541pp/server.hpp>

int main() {
    opcua::ServerConfig config;
    config.setApplicationName("open62541pp server example");
    config.setApplicationUri("urn:open62541pp.server.application");
    config.setProductUri("https://open62541pp.github.io");

    opcua::Server server(std::move(config));

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
