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

struct Login {
    std::string username;
    std::string password;
};

class Server {
public:
    /// Create server with default config
    Server();
    /// Create server with custom port
    Server(uint16_t port);
    /// Create server with custom port and a server certificate
    Server(uint16_t port, std::string_view certificate);

    /// Set custom logger
    void setLogger(Logger logger);

    void setCustomHostname(std::string_view hostname);
    void setApplicationName(std::string_view name);
    void setApplicationUri(std::string_view uri);
    void setProductUri(std::string_view uri);

    void setLogin(const std::vector<Login>& logins, bool allowAnonymous = true);

    void run();
    void stop();
    bool isRunning() const;

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

    UA_Server*       handle();
    const UA_Server* handle() const;

    UA_ServerConfig*       getConfig();
    const UA_ServerConfig* getConfig() const;

private:
    class Connection;
    std::shared_ptr<Connection> connection_;
};

} // namespace opcua
