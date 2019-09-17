#pragma once

#include <memory>
#include <thread>
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
    Server()
        : connection_(std::make_shared<Connection>()) {}

    uint16_t registerNamespace(std::string_view name);

    void setCustomHostname(std::string_view hostname);
    void setApplicationName(std::string_view name);
    void setApplicationUri(std::string_view uri);
    void setProductUri(std::string_view uri);

    void setLogin(const std::vector<Login>& logins, bool allowAnonymous = true);

    inline void start()           { connection_->start(); }
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
private:
    class Connection {
    public:
        Connection();
        ~Connection();

        void start();
        void stop();

        inline bool isRunning() const { return running_.load(); }

        UA_ServerConfig* getConfig();

        inline       UA_Server* handle()       { return server_; }
        inline const UA_Server* handle() const { return server_; }
    private:
        // UA_ServerConfig*  config_;
        UA_Server*        server_;
        std::thread       thread_;
        std::atomic<bool> running_ {false};
    };

    std::shared_ptr<Connection> connection_;
};

} // namespace opcua
