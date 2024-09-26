// This example requires the server example `server_method` to be running.

#include <iostream>

#include "open62541pp/open62541pp.hpp"

int main() {
    opcua::Client client;
    client.connect("opc.tcp://localhost:4840");

    // Browse method node
    opcua::Node objectsNode(client, opcua::ObjectId::ObjectsFolder);
    const opcua::Node greetMethodNode = objectsNode.browseChild({{1, "Greet"}});

    // Call method from parent node (Objects)
    const auto outputs = objectsNode.callMethod(
        greetMethodNode.id(), {opcua::Variant::fromScalar("World")}
    );
    std::cout << outputs.at(0).getScalar<opcua::String>() << std::endl;
}
