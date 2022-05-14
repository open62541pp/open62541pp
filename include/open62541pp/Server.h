#pragma once

#include <memory>
#include <atomic>
#include <string>
#include <string_view>
#include <vector>

// forward declaration open62541
struct UA_Server;
struct UA_ServerConfig;

namespace opcua {

// forward declaration
class NodeId;
class Node;
class ObjectNode;

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

    uint16_t registerNamespace(std::string_view name);

    void setCustomHostname(std::string_view hostname);
    void setApplicationName(std::string_view name);
    void setApplicationUri(std::string_view uri);
    void setProductUri(std::string_view uri);

    void setLogin(const std::vector<Login>& logins, bool allowAnonymous = true);

    inline void run()             { connection_->run(); }
    inline void stop()            { connection_->stop(); }
    inline bool isRunning() const { return connection_->isRunning(); }

    Node       getNode(const NodeId& id);
    ObjectNode getRootNode();
    ObjectNode getObjectsNode();
    ObjectNode getTypesNode();
    ObjectNode getViewsNode();
    ObjectNode getObjectTypesNode();
    ObjectNode getVariableTypesNode();
    ObjectNode getDataTypesNode();
    ObjectNode getReferenceTypesNode();

    void removeNode(const NodeId& id);

    inline       UA_Server* handle()       { return connection_->handle(); }
    inline const UA_Server* handle() const { return connection_->handle(); }

    inline       UA_ServerConfig* getConfig()       { return connection_->getConfig(); }
    inline const UA_ServerConfig* getConfig() const { return connection_->getConfig(); }

private:
    class Connection {
    public:
        Connection();
        ~Connection();

        void run();
        void stop();

        inline bool isRunning() const { return running_.load(); }

        UA_ServerConfig* getConfig();

        inline       UA_Server* handle()       { return server_; }
        inline const UA_Server* handle() const { return server_; }
    private:
        UA_Server*        server_;
        std::atomic<bool> running_ {false};
    };

    std::shared_ptr<Connection> connection_;
};

} // namespace opcua
