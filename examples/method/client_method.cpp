// This example requires the server example `server_method` to be running.

#include <iostream>

#include <open62541pp/client.hpp>
#include <open62541pp/node.hpp>

int main() {
    opcua::Client client;
    client.connect("opc.tcp://localhost:4840");

    // Browse method node
    opcua::Node objectsNode{client, opcua::ObjectId::ObjectsFolder};
    const opcua::Node greetMethodNode = objectsNode.browseChild({{1, "Greet"}});

    // Call method from parent node (Objects)
    const auto result = objectsNode.callMethod(greetMethodNode.id(), {opcua::Variant{"World"}});
    std::cout << result.outputArguments().at(0).scalar<opcua::String>() << std::endl;
}
