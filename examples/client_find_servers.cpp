#include <iostream>
#include <string_view>

#include <open62541pp/client.hpp>

constexpr std::string_view toString(opcua::ApplicationType applicationType) {
    switch (applicationType) {
    case opcua::ApplicationType::Server:
        return "Server";
    case opcua::ApplicationType::Client:
        return "Client";
    case opcua::ApplicationType::ClientAndServer:
        return "Client and Server";
    case opcua::ApplicationType::DiscoveryServer:
        return "Discovery Server";
    default:
        return "Unknown";
    }
}

constexpr std::string_view toString(opcua::MessageSecurityMode securityMode) {
    switch (securityMode) {
    case opcua::MessageSecurityMode::Invalid:
        return "Invalid";
    case opcua::MessageSecurityMode::None:
        return "None";
    case opcua::MessageSecurityMode::Sign:
        return "Sign";
    case opcua::MessageSecurityMode::SignAndEncrypt:
        return "Sign and Encrypt";
    default:
        return "No valid security mode";
    }
}

int main() {
    opcua::Client client;
    client.config().setLogger([](auto&&...) {});  // disable logging

    const auto servers = client.findServers("opc.tcp://localhost:4840");
    size_t serverIndex = 0;
    for (const auto& server : servers) {
        const auto& name = server.applicationUri();
        std::cout
            << "Server[" << serverIndex++ << "] " << name << "\n"
            << "\tName:             " << server.applicationName().text() << "\n"
            << "\tApplication URI:  " << server.applicationUri() << "\n"
            << "\tProduct URI:      " << server.productUri() << "\n"
            << "\tApplication type: " << toString(server.applicationType()) << "\n"
            << "\tDiscovery URLs:\n";

        const auto discoveryUrls = server.discoveryUrls();
        if (discoveryUrls.empty()) {
            std::cout << "No discovery urls provided. Skip endpoint search.";
        }
        for (const auto& url : discoveryUrls) {
            std::cout << "\t- " << url << "\n";
        }

        for (const auto& url : discoveryUrls) {
            size_t endpointIndex = 0;
            for (const auto& endpoint : client.getEndpoints(url)) {
                std::cout
                    << "\tEndpoint[" << endpointIndex++ << "]:\n"
                    << "\t- Endpoint URL:      " << endpoint.endpointUrl() << "\n"
                    << "\t- Transport profile: " << endpoint.transportProfileUri() << "\n"
                    << "\t- Security mode:     " << toString(endpoint.securityMode()) << "\n"
                    << "\t- Security profile:  " << endpoint.securityPolicyUri() << "\n"
                    << "\t- Security level:    " << endpoint.securityLevel() << "\n"
                    << "\t- User identity token:\n";

                for (const auto& token : endpoint.userIdentityTokens()) {
                    std::cout << "\t  - " << token.policyId() << "\n";
                }
            }
        }
    }
}
