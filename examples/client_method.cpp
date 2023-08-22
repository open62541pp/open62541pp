// This example requires the server example `server_method` to be running.

#include <iostream>

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Client client;
    client.connect("opc.tcp://localhost:4840");

    // Browse method node
    auto objNode = client.getObjectsNode();
    auto greetMethodNode = objNode.browseChild({{1, "Greet"}});

    // Call method from parent node (Objects)
    const auto outputs = objNode.callMethod(
        greetMethodNode.getNodeId(), {opcua::Variant::fromScalar("World")}
    );
    std::cout << outputs.at(0).getScalar<opcua::String>() << std::endl;
}
