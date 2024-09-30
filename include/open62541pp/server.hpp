#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "open62541pp/common.hpp"  // NamespaceIndex
#include "open62541pp/config.hpp"
#include "open62541pp/datatype.hpp"
#include "open62541pp/detail/open62541/server.h"
#include "open62541pp/event.hpp"
#include "open62541pp/nodeids.hpp"
#include "open62541pp/session.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/subscription.hpp"
#include "open62541pp/types.hpp"

#include "open62541pp/plugin/accesscontrol.hpp"
#include "open62541pp/plugin/log.hpp"
#include "open62541pp/plugin/log_default.hpp"  // LogFunction
#include "open62541pp/plugin/nodestore.hpp"

namespace opcua {
template <typename Connection>
class Node;
class Server;

namespace detail {
struct ServerConnection;
struct ServerContext;
}  // namespace detail

/* -------------------------------------- Helper functions -------------------------------------- */

namespace detail {

UA_ServerConfig* getConfig(UA_Server* server) noexcept;
UA_ServerConfig& getConfig(Server& server) noexcept;

UA_Logger* getLogger(UA_Server* server) noexcept;
UA_Logger* getLogger(Server& server) noexcept;

ServerConnection* getConnection(UA_Server* server) noexcept;
ServerConnection& getConnection(Server& server) noexcept;

Server* getWrapper(UA_Server* server) noexcept;

ServerContext* getContext(UA_Server* server) noexcept;
ServerContext& getContext(Server& server) noexcept;

}  // namespace detail

/* ------------------------------------------- Server ------------------------------------------- */

/**
 * High-level server class.
 *
 * Exposes the most common functionality. Use the handle() method to get access the underlying
 * UA_Server instance and use the full power of open6254.
 */
class Server {
public:
    /**
     * Create server with default configuration (no encryption).
     * Security policies:
     * - [None](http://opcfoundation.org/UA/SecurityPolicy#None)
     *
     * @param port Port number
     * @param certificate Optional X.509 v3 certificate in `DER` encoded format
     * @param logger Custom log function. If the passed function is empty, the default logger is
     * used.
     */
    explicit Server(uint16_t port = 4840, ByteString certificate = {}, Logger logger = nullptr);

#ifdef UA_ENABLE_ENCRYPTION
    /**
     * Create server with encryption enabled (PKI).
     * Security policies:
     * - [None](http://opcfoundation.org/UA/SecurityPolicy#None)
     * - [Basic128Rsa15](http://opcfoundation.org/UA/SecurityPolicy#Basic128Rsa15)
     * - [Basic256](http://opcfoundation.org/UA/SecurityPolicy#Basic256)
     * - [Basic256Sha256](http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256)
     * - [Aes128_Sha256_RsaOaep](http://opcfoundation.org/UA/SecurityPolicy#Aes128_Sha256_RsaOaep)
     *
     * @param port Port number
     * @param certificate X.509 v3 certificate in `DER` encoded format
     * @param privateKey Private key in `PEM` encoded format
     * @param trustList List of trusted certificates in `DER` encoded format
     * @param issuerList List of issuer certificates (i.e. CAs) in `DER` encoded format
     * @param revocationList Certificate revocation lists (CRL) in `DER` encoded format
     *
     * @see https://reference.opcfoundation.org/Core/Part2/v105/docs/8
     * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/6.1
     * @see https://reference.opcfoundation.org/Core/Part6/v105/docs/6.2
     */
    Server(
        uint16_t port,
        const ByteString& certificate,
        const ByteString& privateKey,
        Span<const ByteString> trustList,
        Span<const ByteString> issuerList,
        Span<const ByteString> revocationList = {}
    );
#endif

    ~Server();

    Server(const Server&) = delete;
    Server(Server&&) noexcept = default;
    Server& operator=(const Server&) = delete;
    Server& operator=(Server&&) noexcept = default;

    /// Set custom logging function.
    /// Does nothing if the passed function is empty or a nullptr.
    void setLogger(Logger logger);

    /// Set custom access control.
    /// @note Supported since open62541 v1.3
    void setAccessControl(AccessControlBase& accessControl);
    /// Set custom access control (transfer ownership to Server).
    /// @note Supported since open62541 v1.3
    void setAccessControl(std::unique_ptr<AccessControlBase> accessControl);

    /// Set custom hostname, default: system's host name.
    [[deprecated("not supported since open62541 v1.4")]]
    void setCustomHostname(std::string_view hostname);
    /// Set application name, default: `open62541-based OPC UA Application`.
    void setApplicationName(std::string_view name);
    /// Set application URI, default: `urn:open62541.server.application`.
    void setApplicationUri(std::string_view uri);
    /// Set product URI, default: `http://open62541.org`.
    void setProductUri(std::string_view uri);

    /// Get active server session.
    /// @note Supported since open62541 v1.3
    std::vector<Session> getSessions();

    /// Get all defined namespaces.
    std::vector<std::string> getNamespaceArray();
    /// Register namespace. The new namespace index will be returned.
    [[nodiscard]] NamespaceIndex registerNamespace(std::string_view uri);

    /// Set custom data types.
    /// All data types provided are automatically considered for decoding of received messages.
    void setCustomDataTypes(std::vector<DataType> dataTypes);

    /// Set value callbacks to execute before every read and after every write operation.
    void setVariableNodeValueCallback(const NodeId& id, ValueCallback callback);
    /// Set data source backend for variable node.
    void setVariableNodeValueBackend(const NodeId& id, ValueBackendDataSource backend);

#ifdef UA_ENABLE_SUBSCRIPTIONS
    /// Create a (pseudo) subscription to monitor local data changes and events.
    Subscription<Server> createSubscription() noexcept;
#endif

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
    /// Create an event object to generate and trigger events.
    [[deprecated("use Event constructor")]]
    Event createEvent(const NodeId& eventType = ObjectTypeId::BaseEventType);
#endif

    /// Run a single iteration of the server's main loop.
    /// @return Maximum wait period until next Server::runIterate call (in ms)
    uint16_t runIterate();
    /// Run the server's main loop. This method will block until Server::stop is called.
    void run();
    /// Stop the server's main loop.
    void stop();
    /// Check if the server is running.
    bool isRunning() const noexcept;

    [[deprecated("use Node constructor")]] Node<Server> getNode(NodeId id);
    [[deprecated("use Node constructor")]] Node<Server> getRootNode();
    [[deprecated("use Node constructor")]] Node<Server> getObjectsNode();
    [[deprecated("use Node constructor")]] Node<Server> getTypesNode();
    [[deprecated("use Node constructor")]] Node<Server> getViewsNode();

    UA_Server* handle() noexcept;
    const UA_Server* handle() const noexcept;

private:
    friend detail::ServerConnection& detail::getConnection(Server& server) noexcept;

    std::unique_ptr<detail::ServerConnection> connection_;
};

inline bool operator==(const Server& lhs, const Server& rhs) noexcept {
    return (lhs.handle() == rhs.handle());
}

inline bool operator!=(const Server& lhs, const Server& rhs) noexcept {
    return !(lhs == rhs);
}

}  // namespace opcua
