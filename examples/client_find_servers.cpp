#include <iostream>
#include <string_view>

#include <open62541pp/client.hpp>

constexpr std::string_view toString(UA_ApplicationType applicationType) {
    switch (applicationType) {
    case UA_APPLICATIONTYPE_SERVER:
        return "Server";
    case UA_APPLICATIONTYPE_CLIENT:
        return "Client";
    case UA_APPLICATIONTYPE_CLIENTANDSERVER:
        return "Client and Server";
    case UA_APPLICATIONTYPE_DISCOVERYSERVER:
        return "Discovery Server";
    default:
        return "Unknown";
    }
}

constexpr std::string_view toString(UA_MessageSecurityMode securityMode) {
    switch (securityMode) {
    case UA_MESSAGESECURITYMODE_INVALID:
        return "Invalid";
    case UA_MESSAGESECURITYMODE_NONE:
        return "None";
    case UA_MESSAGESECURITYMODE_SIGN:
        return "Sign";
    case UA_MESSAGESECURITYMODE_SIGNANDENCRYPT:
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
