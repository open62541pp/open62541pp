#include <iostream>

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Client client;
    client.connect("opc.tcp://localhost:4840");

    opcua::Node node = client.getNode(opcua::VariableId::Server_ServerStatus_CurrentTime);
    const auto dt = node.readValueScalar<opcua::DateTime>();

    std::cout << "Server date (UTC): " << dt.format("%Y-%m-%d %H:%M:%S") << std::endl;
}
