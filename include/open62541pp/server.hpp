#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>  // move
#include <vector>

#include "open62541pp/common.hpp"  // NamespaceIndex
#include "open62541pp/config.hpp"
#include "open62541pp/datatype.hpp"
#include "open62541pp/detail/open62541/server.h"
#include "open62541pp/detail/server_utils.hpp"
#include "open62541pp/session.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/subscription.hpp"  // TODO: remove with Server::createSubscription
#include "open62541pp/types.hpp"
#include "open62541pp/ua/nodeids.hpp"
#include "open62541pp/wrapper.hpp"

#include "open62541pp/plugin/accesscontrol.hpp"
#include "open62541pp/plugin/log.hpp"
#include "open62541pp/plugin/log_default.hpp"  // LogFunction
#include "open62541pp/plugin/nodestore.hpp"

namespace opcua {

/* ---------------------------------------- ServerConfig ---------------------------------------- */

/**
 * Server configuration.
 * @see UA_ServerConfig
 */
class ServerConfig : public Wrapper<UA_ServerConfig> {
public:
    /**
     * Create server config with default configuration.
     * Security policies:
     * - [None](http://opcfoundation.org/UA/SecurityPolicy#None)
     */
    ServerConfig();

    /**
     * Create server config with minimal configuration.
     * Security policies:
     * - [None](http://opcfoundation.org/UA/SecurityPolicy#None)
     *
     * @param port Port number
     * @param certificate Optional X.509 v3 certificate in `DER` encoded format
     */
    explicit ServerConfig(uint16_t port, const ByteString& certificate = {});

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
     * @see https://reference.opcfoundation.org/Core/Part2/v105/docs/9
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

    explicit ServerConfig(UA_ServerConfig&& native);

    ~ServerConfig();

    ServerConfig(const ServerConfig&) = delete;
    ServerConfig(ServerConfig&& other) noexcept;
    ServerConfig& operator=(const ServerConfig&) = delete;
    ServerConfig& operator=(ServerConfig&& other) noexcept;

    void setLogger(LogFunction func);

    void setBuildInfo(BuildInfo buildInfo);

    /// Set application URI, default: `urn:open62541.server.application`.
    void setApplicationUri(std::string_view uri);
    /// Set product URI, default: `http://open62541.org`.
    void setProductUri(std::string_view uri);
    /// Set application name, default: `open62541-based OPC UA Application`.
    void setApplicationName(std::string_view name);

    /// Add custom data types.
    /// All data types provided are automatically considered for decoding of received messages.
    void addCustomDataTypes(Span<const DataType> types);

    /// Set custom access control.
    void setAccessControl(AccessControlBase& accessControl);
    /// Set custom access control (transfer ownership to Server).
    void setAccessControl(std::unique_ptr<AccessControlBase>&& accessControl);
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
 *
 * Don't overwrite the UA_ServerConfig::context pointer! The context pointer is used to store a
 * pointer to the Server instance (for asWrapper(UA_Server*)) and to get access to the underlying
 * server context.
 */
class Server {
public:
    /// Create server with default configuration.
    Server();

    /// Create server with given configuration (move ownership to server).
    explicit Server(ServerConfig&& config);

    /// Create server from native instance (move ownership to server).
    explicit Server(UA_Server* native);

    ~Server();

    Server(const Server&) = delete;
    Server(Server&& other) noexcept;
    Server& operator=(const Server&) = delete;
    Server& operator=(Server&& other) noexcept;

    ServerConfig& config() noexcept;
    const ServerConfig& config() const noexcept;

    [[deprecated("use ServerConfig::addCustomDataTypes via config() or pass config to Server")]]
    void setCustomDataTypes(Span<const DataType> dataTypes) {
        config().addCustomDataTypes(dataTypes);
    }

    /// Get active sessions.
    std::vector<Session> sessions();

    /// Get all defined namespaces.
    std::vector<std::string> namespaceArray();

    /// Register namespace. The new namespace index will be returned.
    [[nodiscard]] NamespaceIndex registerNamespace(std::string_view uri);

    /// Set value callback for variable node.
    void setVariableNodeValueCallback(const NodeId& id, ValueCallbackBase& callback);
    /// Set value callback for variable node (move ownership to server).
    void setVariableNodeValueCallback(
        const NodeId& id, std::unique_ptr<ValueCallbackBase>&& callback
    );
    /// Set data source for variable node.
    void setVariableNodeDataSource(const NodeId& id, DataSourceBase& source);
    /// Set data source for variable node (move ownership to server).
    void setVariableNodeDataSource(const NodeId& id, std::unique_ptr<DataSourceBase>&& source);

#ifdef UA_ENABLE_SUBSCRIPTIONS
    /// Create a (pseudo) subscription to monitor local data changes and events.
    /// @deprecated Use Subscription constructor
    [[deprecated("use Subscription constructor")]]
    Subscription<Server> createSubscription() noexcept;
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

    UA_Server* handle() noexcept;
    const UA_Server* handle() const noexcept;

private:
    detail::ServerContext& context() noexcept;
    const detail::ServerContext& context() const noexcept;

    friend detail::ServerContext& detail::getContext(Server& server) noexcept;

    struct Deleter {
        void operator()(UA_Server* server) noexcept;
    };

    std::unique_ptr<detail::ServerContext> context_;
    std::unique_ptr<UA_Server, Deleter> server_;
};

/// Convert native UA_Server pointer to its wrapper instance.
/// The native server must be owned by a Server instance.
/// @relates Server
Server* asWrapper(UA_Server* server) noexcept;

/// @relates Server
inline bool operator==(const Server& lhs, const Server& rhs) noexcept {
    return (lhs.handle() == rhs.handle());
}

/// @relates Server
inline bool operator!=(const Server& lhs, const Server& rhs) noexcept {
    return !(lhs == rhs);
}

/* -------------------------------------- Async operations -------------------------------------- */

#if UAPP_HAS_ASYNC_OPERATIONS
/// Enable or disable async operations for the specified node.
/// When enabled, operations on the node are queued and must processed by a worker thread using
/// @ref runAsyncOperation.
/// @note Only supported for method nodes.
/// @relates Server
void useAsyncOperation(Server& server, const NodeId& id, bool enabled);

/// Run the next queued async operation (in a worker thread).
/// Async operations are queued by nodes with async operations enabled (see @ref useAsyncOperation).
/// @return `true` if an async operation was processed, `false` if the queue was empty.
/// @relates Server
bool runAsyncOperation(Server& server);
#endif

}  // namespace opcua
