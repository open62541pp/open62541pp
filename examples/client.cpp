#include <iostream>

#include "open62541pp/open62541pp.h"

#include <memory>
#include <vector>

int main() {
    std::shared_ptr<opcua::Client> client(new opcua::Client());


    auto servers = client->findServers("opc.tcp://localhost:4840");
    for (const auto& s : servers)
        std::cout << "Servers count " << s.first << "   " << s.second << "\n";

    auto endpoints = client->getEndpoints("opc.tcp://localhost:4840");
    for (const auto& e : endpoints)
        std::cout << "Endpoints " << e.url << " | " << e.url << "\n";

    client->connect("opc.tcp://localhost:4840");
    opcua::Browser browser(client);
    auto nodes = browser.browse();

    for (auto& n : nodes)
        std::cout << "Node " << n.getBrowseName() << " | " << n.getDisplayName().getText() << "\n";
}