#include <iostream>

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Server server(4840 /* port */);

    server.setApplicationName("open62541pp server example");
    server.setLogger([](auto level, auto category, auto msg) {
        std::cout << "[" << opcua::getLogLevelName(level) << "] "
                  << "[" << opcua::getLogCategoryName(category) << "] " << msg << std::endl;
    });

    // Add a variable node to the Objects node
    auto parentNode = server.getObjectsNode();
    auto myIntegerNode = parentNode.addVariable(
        {1, "the.answer"},
        "the answer",
        opcua::VariableAttributes{}
            .setDataType<int>()
            .setDisplayName({"en-US", "the answer"})
            .setDescription({"en-US", "the answer"})
    );

    // Write a value (attribute) to the node
    myIntegerNode.writeValueScalar(42);

    // Read the value (attribute) from the node
    std::cout << "The answer is: " << myIntegerNode.readValueScalar<int>() << std::endl;

    server.run();
}
