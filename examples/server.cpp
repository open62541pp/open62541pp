#include <iostream>

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Server server(4840 /* port */);

    server.setApplicationName("open62541pp server example");
    server.setLogger([](auto level, auto category, auto msg) {
        std::cout << "[" << opcua::getLogLevelName(level) << "] "
                  << "[" << opcua::getLogCategoryName(category) << "] " << msg << std::endl;
    });

    const opcua::NodeId myIntegerNodeId{1, "the.answer"};
    const std::string myIntegerName{"the answer"};

    // add variable node
    auto parentNode = server.getObjectsNode();
    auto myIntegerNode = parentNode.addVariable(myIntegerNodeId, myIntegerName);

    // set node attributes
    myIntegerNode.setDataType(opcua::Type::Int32);
    myIntegerNode.setDisplayName("en-US", "the answer");
    myIntegerNode.setDescription("en-US", "the answer");

    // write value
    myIntegerNode.writeScalar(42);

    // read value
    std::cout << "The answer is: " << myIntegerNode.readScalar<int>() << std::endl;

    server.run();
}
