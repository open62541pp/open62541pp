#include <iostream>

#include "helper.h"  // CliParser
#include "open62541pp/open62541pp.h"

int main(int argc, char* argv[]) {
    const CliParser parser(argc, argv);
    if (parser.nargs() < 2 || parser.hasFlag("-h") || parser.hasFlag("--help")) {
        std::cout
            << "usage: client_connect [options] opc.tcp://<host>:<port>\n"
            << "options:\n"
            << "  --username <name>\n"
            << "  --password <password>\n"
            << "  --help, -h\n"
            << std::flush;
        return 2;
    }

    const auto serverUrl = parser.args()[parser.nargs() - 1];
    const auto username = parser.getValue("--username");
    const auto password = parser.getValue("--password");

    opcua::Client client;

    if (username) {
        opcua::Login login;
        login.username = username.value();
        login.password = password.value_or("");
        client.connect(serverUrl, login);
    } else {
        client.connect(serverUrl);
    }

    auto node = client.getNode(opcua::VariableId::Server_ServerStatus_CurrentTime);
    const auto dt = node.readValueScalar<opcua::DateTime>();
    client.disconnect();

    std::cout << "Server date (UTC): " << dt.format("%Y-%m-%d %H:%M:%S") << std::endl;
}
