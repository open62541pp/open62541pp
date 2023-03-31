#include <iostream>

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Client client;

    auto servers = client.findServers("opc.tcp://localhost:4840");
    for (const auto& s : servers)
        std::cout << "Servers count " << s.first << "   " << s.second << "\n";

    auto endpoints = client.getEndpoints("opc.tcp://localhost:4840");
    for (const auto& e : endpoints)
        std::cout << "Endpoints " << e.server.productUri << " | " << e.url << "\n";

    // server.setApplicationName("open62541pp server example");
    // server.setLogger([](auto level, auto category, auto msg) {
    //     std::cout << "[" << opcua::getLogLevelName(level) << "] "
    //               << "[" << opcua::getLogCategoryName(category) << "] " << msg << std::endl;
    // });

    // const opcua::NodeId myIntegerNodeId{"the.answer", 1};
    // const std::string myIntegerName{"the answer"};

    // // add variable node
    // auto parentNode = server.getObjectsNode();
    // auto myIntegerNode = parentNode.addVariable(myIntegerNodeId, myIntegerName);

    // // set node attributes
    // myIntegerNode.setDataType(opcua::Type::Int32);
    // myIntegerNode.setDisplayName("the answer", "en-US");
    // myIntegerNode.setDescription("the answer", "en-US");

    // // write value
    // myIntegerNode.writeScalar(42);

    // read value
    // std::cout << "The answer is: " << myIntegerNode.readScalar<int>() << std::endl; //
}