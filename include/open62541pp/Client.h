#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "open62541pp/Auth.h"

// forward declaration open62541
struct UA_Client;

namespace opcua {

// forward declaration
class NodeId;
class ApplicationDescription;
class EndpointDescription;

template <typename ServerOrClient>
class Node;

/**
 * High-level client class.
 *
 * Exposes the most common functionality. Use the handle() and getConfig() method to get access
 * to the underlying UA_Client and UA_ClientConfig instances and use the full power of open6254.
 */
class Client {
public:
    /**
     * Create client with default configuration.
     */
    Client();

    /**
     * Gets a list of all registered servers at the given server.
     * @param serverUrl url to connect (for example `opc.tcp://localhost:4840`)
     */
    std::vector<ApplicationDescription> findServers(std::string_view serverUrl);

    /**
     * Gets a list of endpoints of a server.
     * @param serverUrl url to connect (for example `opc.tcp://localhost:4840`)
     */
    std::vector<EndpointDescription> getEndpoints(std::string_view serverUrl);

    /**
     * Connect to the selected server.
     * @param endpointUrl to connect (for example `opc.tcp://localhost:4840/open62541/server/`)
     */
    void connect(std::string_view endpointUrl);

    /**
     * Connect to the selected server with the given username and password.
     * @param endpointUrl to connect (for example `opc.tcp://localhost:4840/open62541/server/`)
     * @param login credentials with username and password
     */
    void connect(std::string_view endpointUrl, const Login& login);

    Node<Client> getNode(const NodeId& id);
    Node<Client> getRootNode();
    Node<Client> getObjectsNode();
    Node<Client> getTypesNode();
    Node<Client> getViewsNode();
    Node<Client> getObjectTypesNode();
    Node<Client> getVariableTypesNode();
    Node<Client> getDataTypesNode();
    Node<Client> getReferenceTypesNode();
    Node<Client> getBaseObjectTypeNode();
    Node<Client> getBaseDataVariableTypeNode();

    UA_Client* handle() noexcept;
    const UA_Client* handle() const noexcept;

private:
    class Connection;
    std::shared_ptr<Connection> connection_;
};

}  // namespace opcua
