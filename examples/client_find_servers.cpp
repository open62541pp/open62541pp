#include <iostream>
#include <string_view>

#include "open62541pp/open62541pp.h"

constexpr std::string_view getEnumName(UA_ApplicationType applicationType) {
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

constexpr std::string_view getEnumName(UA_MessageSecurityMode securityMode) {
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
    client.setLogger({});  // disable logging

    const auto servers = client.findServers("opc.tcp://localhost:4840");
    size_t serverIndex = 0;
    for (const auto& server : servers) {
        const auto& name = server.getApplicationUri();
        std::cout
            << "Server[" << serverIndex++ << "] " << name << "\n"
            << "\tName:             " << server.getApplicationName().getText() << "\n"
            << "\tApplication URI:  " << server.getApplicationUri() << "\n"
            << "\tProduct URI:      " << server.getProductUri() << "\n"
            << "\tApplication type: " << getEnumName(server.getApplicationType()) << "\n"
            << "\tDiscovery URLs:\n";

        const auto discoveryUrls = server.getDiscoveryUrls();
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
                    << "\t- Endpoint URL:      " << endpoint.getEndpointUrl() << "\n"
                    << "\t- Transport profile: " << endpoint.getTransportProfileUri() << "\n"
                    << "\t- Security mode:     " << getEnumName(endpoint.getSecurityMode()) << "\n"
                    << "\t- Security profile:  " << endpoint.getSecurityPolicyUri() << "\n"
                    << "\t- Security level:    " << endpoint.getSecurityLevel() << "\n"
                    << "\t- User identity token:\n";

                for (const auto& token : endpoint.getUserIdentityTokens()) {
                    std::cout << "\t  - " << token.getPolicyId() << "\n";
                }
            }
        }
    }
}
