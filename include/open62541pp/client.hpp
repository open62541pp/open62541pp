#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "open62541pp/config.hpp"
#include "open62541pp/datatype.hpp"
#include "open62541pp/detail/open62541/client.h"
#include "open62541pp/span.hpp"
#include "open62541pp/subscription.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/types_composed.hpp"

#include "open62541pp/plugin/log.hpp"
#include "open62541pp/plugin/log_default.hpp"  // LogFunction

namespace opcua {
class Client;
struct Login;
template <typename Connection>
class Node;

namespace detail {
struct ClientConnection;
struct ClientContext;
}  // namespace detail

/* -------------------------------------- Helper functions -------------------------------------- */

namespace detail {

UA_ClientConfig* getConfig(UA_Client* client) noexcept;
UA_ClientConfig& getConfig(Client& client) noexcept;

UA_Logger* getLogger(UA_Client* client) noexcept;
UA_Logger* getLogger(Client& client) noexcept;

ClientConnection* getConnection(UA_Client* client) noexcept;
ClientConnection& getConnection(Client& client) noexcept;

Client* getWrapper(UA_Client* client) noexcept;

ClientContext* getContext(UA_Client* client) noexcept;
ClientContext& getContext(Client& client) noexcept;

}  // namespace detail

/* ------------------------------------------- Client ------------------------------------------- */

using StateCallback = std::function<void()>;
using InactivityCallback = std::function<void()>;

/**
 * High-level client class.
 *
 * Exposes the most common functionality. Use the handle() method to get access the underlying
 * UA_Client instance and use the full power of open6254.
 */
class Client {
public:
    /**
     * Create client with default configuration (no encryption).
     * Security policies:
     * - [None](http://opcfoundation.org/UA/SecurityPolicy#None)
     * @param logger Custom log function. If the passed function is empty, the default logger is
     * used.
     */
    explicit Client(LogFunction logger = nullptr);

#ifdef UA_ENABLE_ENCRYPTION
    /**
     * Create client with encryption enabled (PKI).
     * Security policies:
     * - [None](http://opcfoundation.org/UA/SecurityPolicy#None)
     * - [Basic128Rsa15](http://opcfoundation.org/UA/SecurityPolicy#Basic128Rsa15)
     * - [Basic256](http://opcfoundation.org/UA/SecurityPolicy#Basic256)
     * - [Basic256Sha256](http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256)
     * - [Aes128_Sha256_RsaOaep](http://opcfoundation.org/UA/SecurityPolicy#Aes128_Sha256_RsaOaep)
     *
     * @param certificate X.509 v3 certificate in `DER` encoded format
     * @param privateKey Private key in `PEM` encoded format
     * @param trustList List of trusted certificates in `DER` encoded format
     * @param revocationList Certificate revocation lists (CRL) in `DER` encoded format
     *
     * @see https://reference.opcfoundation.org/Core/Part2/v105/docs/8
     * @see https://reference.opcfoundation.org/Core/Part6/v105/docs/6.2
     */
    Client(
        const ByteString& certificate,
        const ByteString& privateKey,
        Span<const ByteString> trustList,
        Span<const ByteString> revocationList = {}
    );
#endif

    ~Client();

    Client(const Client&) = delete;
    Client(Client&&) noexcept = default;
    Client& operator=(const Client&) = delete;
    Client& operator=(Client&&) noexcept = default;

    /**
     * Gets a list of all registered servers at the given server.
     * @param serverUrl Server URL (for example `opc.tcp://localhost:4840`)
     * @note Client must be disconnected.
     */
    std::vector<ApplicationDescription> findServers(std::string_view serverUrl);

    /**
     * Gets a list of endpoints of a server.
     * @copydetails findServers
     */
    std::vector<EndpointDescription> getEndpoints(std::string_view serverUrl);

    /// Set custom log function.
    /// Does nothing if the passed function is empty or a nullptr.
    void setLogger(LogFunction logger);

    /// Set response timeout in milliseconds.
    void setTimeout(uint32_t milliseconds);

    /// Set anonymous identity token.
    void setUserIdentityToken(AnonymousIdentityToken token);
    /// Set username/password identity token.
    void setUserIdentityToken(UserNameIdentityToken token);
    /// Set X.509 identity token.
    void setUserIdentityToken(X509IdentityToken token);
    /// Set issued identity token.
    void setUserIdentityToken(IssuedIdentityToken token);

    /// Set message security mode.
    void setSecurityMode(MessageSecurityMode mode);

    /// Set custom data types.
    /// All data types provided are automatically considered for decoding of received messages.
    void setCustomDataTypes(std::vector<DataType> dataTypes);

    /// Set a state callback that will be called after the client is connected.
    void onConnected(StateCallback callback);
    /// Set a state callback that will be called after the client is disconnected.
    void onDisconnected(StateCallback callback);
    /// Set a state callback that will be called after the session is activated.
    void onSessionActivated(StateCallback callback);
    /// Set a state callback that will be called after the session is closed.
    void onSessionClosed(StateCallback callback);
    /// Set an inactivity callback.
    /// Every UA_ClientConfig::connectivityCheckInterval (in ms), an async read request is performed
    /// on the server. The callback is called when the client receives no response for this request.
    void onInactive(InactivityCallback callback);

    /**
     * Connect to the selected server.
     * The session authentification method is defined by the UserIdentityToken and is set with
     * Client::setUserIdentityToken.
     * @param endpointUrl Endpoint URL (for example `opc.tcp://localhost:4840/open62541/server/`)
     */
    void connect(std::string_view endpointUrl);

    /**
     * Connect to the selected server with the given username and password.
     * @param endpointUrl Endpoint URL (for example `opc.tcp://localhost:4840/open62541/server/`)
     * @param login       Login credentials with username and password
     */
    [[deprecated("use Client::setUserIdentityToken(UserNameIdentityToken) instead")]]
    void connect(std::string_view endpointUrl, const Login& login);

    /// Disconnect and close a connection to the server (async, without blocking).
    void disconnect() noexcept;

    /// Check if client is connected (secure channel open).
    bool isConnected() noexcept;

    /// Get all defined namespaces.
    std::vector<std::string> getNamespaceArray();

#ifdef UA_ENABLE_SUBSCRIPTIONS
    /// Create a subscription to monitor data changes and events (default subscription parameters).
    Subscription<Client> createSubscription();
    /// Create a subscription to monitor data changes and events.
    Subscription<Client> createSubscription(SubscriptionParameters& parameters);
    /// Get all active subscriptions
    std::vector<Subscription<Client>> getSubscriptions();
#endif

    /**
     * Run a single iteration of the client's main loop.
     * Listen on the network and process arriving asynchronous responses in the background.
     * Internal housekeeping, renewal of SecureChannels and subscription management is done as well.
     * @param timeoutMilliseconds Timeout in milliseconds
     */
    void runIterate(uint16_t timeoutMilliseconds = 1000);
    /// Run the client's main loop by. This method will block until Client::stop is called.
    void run();
    /// Stop the client's main loop.
    void stop();
    /// Check if the client's main loop is running.
    bool isRunning() const noexcept;

    [[deprecated("use Node constructor")]] Node<Client> getNode(NodeId id);
    [[deprecated("use Node constructor")]] Node<Client> getRootNode();
    [[deprecated("use Node constructor")]] Node<Client> getObjectsNode();
    [[deprecated("use Node constructor")]] Node<Client> getTypesNode();
    [[deprecated("use Node constructor")]] Node<Client> getViewsNode();

    UA_Client* handle() noexcept;
    const UA_Client* handle() const noexcept;

private:
    friend detail::ClientConnection& detail::getConnection(Client& client) noexcept;

    std::unique_ptr<detail::ClientConnection> connection_;
};

inline bool operator==(const Client& lhs, const Client& rhs) noexcept {
    return (lhs.handle() == rhs.handle());
}

inline bool operator!=(const Client& lhs, const Client& rhs) noexcept {
    return !(lhs == rhs);
}

}  // namespace opcua
