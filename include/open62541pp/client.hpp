#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>  // move
#include <vector>

#include "open62541pp/config.hpp"
#include "open62541pp/datatype.hpp"
#include "open62541pp/detail/client_utils.hpp"
#include "open62541pp/detail/open62541/client.h"
#include "open62541pp/span.hpp"
#include "open62541pp/subscription.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/ua/types.hpp"
#include "open62541pp/wrapper.hpp"

#include "open62541pp/plugin/log.hpp"
#include "open62541pp/plugin/log_default.hpp"  // LogFunction

namespace opcua {
struct Login;
template <typename Connection>
class Node;

/* ---------------------------------------- ClientConfig ---------------------------------------- */

/**
 * Client configuration.
 * @see UA_ClientConfig
 */
class ClientConfig : public Wrapper<UA_ClientConfig> {
public:
    /**
     * Create client config with default configuration (no encryption).
     * Security policies:
     * - [None](http://opcfoundation.org/UA/SecurityPolicy#None)
     */
    ClientConfig();

#ifdef UA_ENABLE_ENCRYPTION
    /**
     * Create client config with encryption enabled (PKI).
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
     * @see https://reference.opcfoundation.org/Core/Part2/v105/docs/9
     * @see https://reference.opcfoundation.org/Core/Part6/v105/docs/6.2
     */
    ClientConfig(
        const ByteString& certificate,
        const ByteString& privateKey,
        Span<const ByteString> trustList,
        Span<const ByteString> revocationList = {}
    );
#endif

    explicit ClientConfig(UA_ClientConfig&& native);

    ~ClientConfig();

    ClientConfig(const ClientConfig&) = delete;
    ClientConfig(ClientConfig&& other) noexcept;
    ClientConfig& operator=(const ClientConfig&) = delete;
    ClientConfig& operator=(ClientConfig&& other) noexcept;

    /// Set custom log function.
    /// Does nothing if the passed function is empty or a nullptr.
    void setLogger(LogFunction func);

    /// Set response timeout in milliseconds.
    void setTimeout(uint32_t milliseconds) noexcept;

    /// Set anonymous identity token.
    void setUserIdentityToken(const AnonymousIdentityToken& token);
    /// Set username/password identity token.
    void setUserIdentityToken(const UserNameIdentityToken& token);
    /// Set X.509 identity token.
    void setUserIdentityToken(const X509IdentityToken& token);
    /// Set issued identity token.
    void setUserIdentityToken(const IssuedIdentityToken& token);

    /// Set message security mode.
    void setSecurityMode(MessageSecurityMode mode) noexcept;
};

/* ------------------------------------------- Client ------------------------------------------- */

using StateCallback = std::function<void()>;
using InactivityCallback = std::function<void()>;
using SubscriptionInactivityCallback = std::function<void(IntegerId subscriptionId)>;

/**
 * High-level client class.
 *
 * A client is usually created in two steps:
 * 1. Create and modify a client configuration (ClientConfig)
 * 2. Create a Client with a ClientConfig (move ownership to the Client instance)
 *
 * Once a client is connected to an `endpointUrl`, it is not possible to switch to another server.
 * A new client has to be created for that.
 *
 * Use the handle() method to get access the underlying UA_Server instance and use the full power
 * of open62541.
 *
 * Don't overwrite the UA_ClientConfig::clientContext pointer! The context pointer is used to store
 * a pointer to the Client instance (for asWrapper(UA_Client*)) and to get access to the underlying
 * client context.
 */
class Client {
public:
    /// Create client with default configuration.
    Client();

    /// Create client with given configuration (move ownership to client).
    explicit Client(ClientConfig&& config);

#ifdef UA_ENABLE_ENCRYPTION
    /// @copydoc ClientConfig::ClientConfig(
    ///     const ByteString&, const ByteString&, Span<const ByteString>, Span<const ByteString>
    /// )
    [[deprecated("use ClientConfig constructor and construct Client with ClientConfig")]]
    Client(
        const ByteString& certificate,
        const ByteString& privateKey,
        Span<const ByteString> trustList,
        Span<const ByteString> revocationList = {}
    );
#endif

    ~Client();

    Client(const Client&) = delete;
    Client(Client&& other) noexcept;
    Client& operator=(const Client&) = delete;
    Client& operator=(Client&& other) noexcept;

    ClientConfig& config() noexcept;
    const ClientConfig& config() const noexcept;

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

    [[deprecated("use ClientConfig::setLogger via config() or pass config to Client")]]
    void setLogger(LogFunction logger) {
        config().setLogger(std::move(logger));
    }

    [[deprecated("use ClientConfig::setTimeout via config() or pass config to Client")]]
    void setTimeout(uint32_t milliseconds) noexcept {
        config().setTimeout(milliseconds);
    }

    template <typename Token>
    [[deprecated("use ClientConfig::setUserIdentityToken via config() or pass config to Client")]]
    void setUserIdentityToken(const Token& token) {
        config().setUserIdentityToken(token);
    }

    [[deprecated("use ClientConfig::setSecurityMode via config() or pass config to Client")]]
    void setSecurityMode(MessageSecurityMode mode) noexcept {
        config().setSecurityMode(mode);
    }

    /// Set custom data types.
    /// All data types provided are automatically considered for decoding of received messages.
    void setCustomDataTypes(Span<const DataType> dataTypes);

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
    /// Set a subscription inactivity callback.
    /// The callback is called when the client does not receive a publish response after the defined
    /// delay of `(publishingInterval * maxKeepAliveCount) + UA_ClientConfig::timeout)`.
    void onSubscriptionInactive(SubscriptionInactivityCallback callback);

    /**
     * Connect to the selected server.
     * The session authentification method is defined by the UserIdentityToken and is set with
     * Client::setUserIdentityToken.
     * @param endpointUrl Endpoint URL (for example `opc.tcp://localhost:4840/open62541/server/`)
     */
    void connect(std::string_view endpointUrl);

    /**
     * Asynchronously connect to the selected server.
     * Set a state callback, e.g. onConnected, to be notified when the connection status changes.
     * @copydetails connect
     */
    void connectAsync(std::string_view endpointUrl);

    /**
     * Connect to the selected server with the given username and password.
     * @param endpointUrl Endpoint URL (for example `opc.tcp://localhost:4840/open62541/server/`)
     * @param login       Login credentials with username and password
     */
    [[deprecated("use Client::setUserIdentityToken(UserNameIdentityToken) instead")]]
    void connect(std::string_view endpointUrl, const Login& login);

    /**
     * Disconnect and close the connection to the server.
     */
    void disconnect();

    /**
     * Asynchronously disconnect and close the connection to the server.
     * Set a state callback, e.g. onDisconnected, to be notified when the connection status changes.
     */
    void disconnectAsync();

    /// Check if client is connected (secure channel open).
    bool isConnected() noexcept;

    /// Get all defined namespaces.
    std::vector<std::string> namespaceArray();

    /// @deprecated Use namespaceArray() instead
    [[deprecated("use namespaceArray() instead")]]
    std::vector<std::string> getNamespaceArray() {
        return namespaceArray();
    }

#ifdef UA_ENABLE_SUBSCRIPTIONS
    /// Create a subscription to monitor data changes and events.
    Subscription<Client> createSubscription(const SubscriptionParameters& parameters = {});
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
    detail::ClientContext& context() noexcept;
    const detail::ClientContext& context() const noexcept;

    friend detail::ClientContext& detail::getContext(Client& client) noexcept;

    struct Deleter {
        void operator()(UA_Client* client) noexcept;
    };

    std::unique_ptr<detail::ClientContext> context_;
    std::unique_ptr<UA_Client, Deleter> client_;
};

/// Convert native UA_Client pointer to its wrapper instance.
/// The native client must be owned by a Client instance.
Client* asWrapper(UA_Client* client) noexcept;

inline bool operator==(const Client& lhs, const Client& rhs) noexcept {
    return (lhs.handle() == rhs.handle());
}

inline bool operator!=(const Client& lhs, const Client& rhs) noexcept {
    return !(lhs == rhs);
}

}  // namespace opcua
