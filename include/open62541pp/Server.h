#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "open62541pp/Auth.h"
#include "open62541pp/Logger.h"
#include "open62541pp/types/NodeId.h"

// forward declaration open62541
struct UA_Server;

namespace opcua {

// forward declaration
struct ServerContext;
template <typename ServerOrClient>
class Node;

/**
 * High-level server class.
 *
 * Exposes the most common functionality. Use the handle() method to get access the underlying
 * UA_Server instance and use the full power of open6254.
 */
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

    /// Get all defined namespaces.
    std::vector<std::string> getNamespaceArray();
    /// Register namespace. The new namespace index will be returned.
    [[nodiscard]] uint16_t registerNamespace(std::string_view uri);

    /// Run single iteration of the server's main loop.
    /// @returns Maximum wait period until next Server::runIterate call (in ms)
    uint16_t runIterate();
    /// Run the server's main loop. This method will block until Server::stop is called.
    void run();
    /// Stop the server's main loop.
    void stop();
    /// Check if the server is running.
    bool isRunning() const noexcept;

    Node<Server> getNode(const NodeId& id);
    Node<Server> getRootNode();
    Node<Server> getObjectsNode();
    Node<Server> getTypesNode();
    Node<Server> getViewsNode();

    UA_Server* handle() noexcept;
    const UA_Server* handle() const noexcept;

    /// Get client context (for internal use only).
    /// @private
    ServerContext& getContext() noexcept;

private:
    class Connection;
    std::shared_ptr<Connection> connection_;
};

/* ---------------------------------------------------------------------------------------------- */

bool operator==(const Server& left, const Server& right) noexcept;
bool operator!=(const Server& left, const Server& right) noexcept;

}  // namespace opcua
