#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "open62541pp/Config.h"
#include "open62541pp/Logger.h"
#include "open62541pp/NodeIds.h"
#include "open62541pp/Span.h"
#include "open62541pp/Subscription.h"
#include "open62541pp/types/NodeId.h"

// forward declaration open62541
struct UA_Server;

namespace opcua {

// forward declaration
class AccessControlBase;
class ByteString;
class DataType;
class Event;
template <typename ServerOrClient>
class Node;
class ServerContext;
class Session;
struct ValueBackendDataSource;
struct ValueCallback;

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
     */
    explicit Server(uint16_t port = 4840, ByteString certificate = {});

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

    /// Set custom logging function.
    void setLogger(Logger logger);

    /// Set custom access control.
    void setAccessControl(AccessControlBase& accessControl);
    /// Set custom access control (transfer ownership to Server).
    void setAccessControl(std::unique_ptr<AccessControlBase> accessControl);

    /// Set custom hostname, default: system's host name.
    void setCustomHostname(std::string_view hostname);
    /// Set application name, default: `open62541-based OPC UA Application`.
    void setApplicationName(std::string_view name);
    /// Set application URI, default: `urn:open62541.server.application`.
    void setApplicationUri(std::string_view uri);
    /// Set product URI, default: `http://open62541.org`.
    void setProductUri(std::string_view uri);

    /// Get active client session.
    std::vector<Session> getSessions() const;

    /// Get all defined namespaces.
    std::vector<std::string> getNamespaceArray();
    /// Register namespace. The new namespace index will be returned.
    [[nodiscard]] uint16_t registerNamespace(std::string_view uri);

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
    Event createEvent(const NodeId& eventType = ObjectTypeId::BaseEventType);
#endif

    /// Run a single iteration of the server's main loop.
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

bool operator==(const Server& lhs, const Server& rhs) noexcept;
bool operator!=(const Server& lhs, const Server& rhs) noexcept;

}  // namespace opcua
