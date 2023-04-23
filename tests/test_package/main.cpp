#include <iostream>

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Client client;

    try {
        client.connect("opc.tcp://localhost:4840");
        auto node = client.getNode({0, UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME});
        const auto dt = node.readScalar<opcua::DateTime>();

        std::cout << "Server date (UTC): " << dt.format("%Y-%m-%d %H:%M:%S") << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Client failed with exception: "<< e.what() << std::endl;
    }


}
