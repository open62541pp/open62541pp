#include <iostream>

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Server server;
    server.setApplicationName("open62541pp server example");

    server.setLogger([](auto level, auto category, auto msg) {
        std::cout
            << "[" << opcua::getLogLevelName(level) << "] "
            << "[" << opcua::getLogCategoryName(category) << "] "
            << msg << std::endl;
    });

    const auto        myIntegerNodeId = opcua::NodeId("the.answer", 1);
    const std::string myIntegerName   = "the answer";

    // create node
    auto parentNode    = server.getObjectsNode();
    auto myIntegerNode = parentNode.addVariable(myIntegerNodeId, myIntegerName, opcua::Type::Int32);

    // set node attributes
    myIntegerNode.setDisplayName("the answer");
    myIntegerNode.setDescription("the answer");

    // write value
    myIntegerNode.write(42);

    // read value
    std::cout << "The answer is: " << myIntegerNode.read<int>() << std::endl;

    server.run();

    return 0;
}
