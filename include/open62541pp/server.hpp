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

UA_Logger* getLogger(UA_ServerConfig* config) noexcept;

ServerConnection* getConnection(UA_Server* server) noexcept;
ServerConnection& getConnection(Server& server) noexcept;

Server* getWrapper(UA_Server* server) noexcept;

ServerContext* getContext(UA_Server* server) noexcept;
ServerContext& getContext(Server& server) noexcept;

}  // namespace detail

/* ---------------------------------------- ServerConfig ---------------------------------------- */

/**
 * Server configuration.
 * @see UA_ServerConfig
 */
class ServerConfig {
public:
    /**
     * Create server config with default configuration (no encryption).
     * Security policies:
     * - [None](http://opcfoundation.org/UA/SecurityPolicy#None)
     *
     * @param port Port number
     * @param certificate Optional X.509 v3 certificate in `DER` encoded format
     */
    explicit ServerConfig(uint16_t port = 4840, const ByteString& certificate = {});

#ifdef UA_ENABLE_ENCRYPTION
    /**
     * Create server config with encryption enabled (PKI).
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
    ServerConfig(
        uint16_t port,
        const ByteString& certificate,
        const ByteString& privateKey,
        Span<const ByteString> trustList,
        Span<const ByteString> issuerList,
        Span<const ByteString> revocationList = {}
    );
#endif

    explicit ServerConfig(UA_ServerConfig&& config);

    ~ServerConfig();

    ServerConfig(const ServerConfig&) = delete;
    ServerConfig(ServerConfig&&) noexcept = default;
    ServerConfig& operator=(const ServerConfig&) = delete;
    ServerConfig& operator=(ServerConfig&&) noexcept = default;

    UA_ServerConfig* operator->() noexcept;
    const UA_ServerConfig* operator->() const noexcept;

    /// Set custom log function.
    /// Does nothing if the passed function is empty or a nullptr.
    void setLogger(LogFunction logger);

    /// Set application name, default: `open62541-based OPC UA Application`.
    void setApplicationName(std::string_view name);
    /// Set application URI, default: `urn:open62541.server.application`.
    void setApplicationUri(std::string_view uri);
    /// Set product URI, default: `http://open62541.org`.
    void setProductUri(std::string_view uri);

    /// Set custom data types.
    /// All data types provided are automatically considered for decoding of received messages.
    void setCustomDataTypes(std::vector<DataType> types);

    /// Set custom access control.
    void setAccessControl(AccessControlBase& accessControl);

    UA_ServerConfig* handle() noexcept;
    const UA_ServerConfig* handle() const noexcept;

private:
    friend struct detail::ServerConnection;

    detail::ServerContext& context() noexcept;

    // TODO: remove workaround with external config & context (2nd variant alternative)
    ServerConfig(UA_ServerConfig& config, detail::ServerContext& context);
    std::variant<UA_ServerConfig, UA_ServerConfig*> config_;
    std::variant<std::unique_ptr<detail::ServerContext>, detail::ServerContext*> context_;
};

/* ------------------------------------------- Server ------------------------------------------- */

/**
 * High-level server class.
 *
 * A server is usually created in two steps:
 * 1. Create and modify a server configuration (ServerConfig)
 * 2. Create a Server with a ServerConfig (move ownership to the Server instance)
 *
 * The server expects that the configuration is not modified during runtime.
 *
 * Use the handle() method to get access the underlying UA_Server instance and use the full power
 * of open62541.
 */
class Server {
public:
    /// Create server with default configuration.
    Server();

    /// Create server with given configuration (move ownership to server).
    explicit Server(ServerConfig&& config);

    /// @copydoc ServerConfig::ServerConfig(uint16_t, const ByteString&)
    [[deprecated("use ServerConfig constructor and construct Server with ServerConfig")]]
    explicit Server(uint16_t port, const ByteString& certificate = {});

#ifdef UA_ENABLE_ENCRYPTION
    /// @copydoc ServerConfig::ServerConfig(
    ///     uint16_t,
    ///     const ByteString&,
    ///     const ByteString&,
    ///     Span<const ByteString>,
    ///     Span<const ByteString>,
    ///     Span<const ByteString>
    /// )
    [[deprecated("use ServerConfig constructor an pass config to Server constructor")]]
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

    /// @copydoc ServerConfig::setLogger
    [[deprecated("use ServerConfig::setLogger and pass config to Server constructor")]]
    void setLogger(LogFunction logger);

    /// Set custom hostname, default: system's host name.
    [[deprecated("not supported since open62541 v1.4")]]
    void setCustomHostname(std::string_view hostname);
    /// @copydoc ServerConfig::setApplicationName
    [[deprecated("use ServerConfig::setApplicationName and pass config to Server constructor")]]
    void setApplicationName(std::string_view name);
    /// @copydoc ServerConfig::setApplicationUri
    [[deprecated("use ServerConfig::setApplicationUri and pass config to Server constructor")]]
    void setApplicationUri(std::string_view uri);
    /// @copydoc ServerConfig::setProductUri
    [[deprecated("use ServerConfig::setProductUri and pass config to Server constructor")]]
    void setProductUri(std::string_view uri);

    /// @copydoc ServerConfig::setCustomDataTypes
    [[deprecated("use ServerConfig::setCustomDataTypes and pass config to Server constructor")]]
    void setCustomDataTypes(std::vector<DataType> dataTypes);

    /// @copydoc ServerConfig::setAccessControl
    [[deprecated("use ServerConfig::setAccessControl and pass config to Server constructor")]]
    void setAccessControl(AccessControlBase& accessControl);
    [[deprecated("use ServerConfig::setAccessControl and pass config to Server constructor")]]
    void setAccessControl(std::unique_ptr<AccessControlBase> accessControl);

    /// Get active server session.
    std::vector<Session> getSessions();

    /// Get all defined namespaces.
    std::vector<std::string> getNamespaceArray();
    /// Register namespace. The new namespace index will be returned.
    [[nodiscard]] NamespaceIndex registerNamespace(std::string_view uri);

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
