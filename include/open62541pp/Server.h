#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "open62541pp/Logger.h"

// forward declaration open62541
struct UA_Server;
struct UA_ServerConfig;

namespace opcua {

// forward declaration
class NodeId;
class Node;

/// Login credentials.
struct Login {
    std::string username;
    std::string password;
};

class Server {
public:
    /// Create server with default config.
    Server();
    /// Create server with custom port.
    explicit Server(uint16_t port);
    /// Create server with custom port and a server certificate.
    Server(uint16_t port, std::string_view certificate);

    /// Set custom logging function.
    void setLogger(Logger logger);
    /// Set custom hostname, default: system's host name.
    void setCustomHostname(std::string_view hostname);
    /// Set application name, default: `open62541-based OPC UA Application`.
    void setApplicationName(std::string_view name);
    /// Set application URI, default: `urn:open62541.server.application`.
    void setApplicationUri(std::string_view uri);
    /// Set product URI, default: `http://open62541.org`.
    void setProductUri(std::string_view uri);

    /// Set login credentials (username/password) and anonymous login.
    void setLogin(const std::vector<Login>& logins, bool allowAnonymous = true);

    /// Run server. This method will block until Server::stop is called.
    void run();
    /// Stop server.
    void stop();
    /// Check if server is running.
    bool isRunning() const;

    /// Register namespace. The new namespace index will be returned.
    uint16_t registerNamespace(std::string_view name);

    Node getNode(const NodeId& id);
    Node getRootNode();
    Node getObjectsNode();
    Node getTypesNode();
    Node getViewsNode();
    Node getObjectTypesNode();
    Node getVariableTypesNode();
    Node getDataTypesNode();
    Node getReferenceTypesNode();

    void removeNode(const NodeId& id);

    UA_Server* handle();
    const UA_Server* handle() const;

    UA_ServerConfig* getConfig();
    const UA_ServerConfig* getConfig() const;

private:
    class Connection;
    std::shared_ptr<Connection> connection_;
};

}  // namespace opcua
