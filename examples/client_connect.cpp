#include <iostream>

#include <open62541pp/client.hpp>
#include <open62541pp/node.hpp>

#include "helper.hpp"  // CliParser

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
        client.config().setUserIdentityToken(
            opcua::UserNameIdentityToken(username.value(), password.value_or(""))
        );
    }
    client.connect(serverUrl);

    opcua::Node node(client, opcua::VariableId::Server_ServerStatus_CurrentTime);
    const auto dt = node.readValueScalar<opcua::DateTime>();
    client.disconnect();

    std::cout << "Server date (UTC): " << dt.format("%Y-%m-%d %H:%M:%S") << std::endl;
}
