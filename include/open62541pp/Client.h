#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "open62541pp/Auth.h"
#include "open62541pp/Config.h"
#include "open62541pp/Logger.h"
#include "open62541pp/Subscription.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/NodeId.h"

// forward declaration open62541
struct UA_Client;

namespace opcua {

// forward declaration
class ClientContext;
template <typename ServerOrClient>
class Node;

using StateCallback = std::function<void()>;

/**
 * High-level client class.
 *
 * Exposes the most common functionality. Use the handle() method to get access the underlying
 * UA_Client instance and use the full power of open6254.
 */
class Client {
public:
    /// Create client with default configuration.
    Client();

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

    /// Set custom logging function.
    void setLogger(Logger logger);

    /// Set response timeout in milliseconds.
    void setTimeout(uint32_t milliseconds);

    /// Set a state callback that will be called after the client is connected.
    void onConnected(StateCallback callback);
    /// Set a state callback that will be called after the client is disconnected.
    void onDisconnected(StateCallback callback);
    /// Set a state callback that will be called after the session is activated.
    void onSessionActivated(StateCallback callback);
    /// Set a state callback that will be called after the session is closed.
    void onSessionClosed(StateCallback callback);

    /**
     * Connect to the selected server.
     * @param endpointUrl Endpoint URL (for example `opc.tcp://localhost:4840/open62541/server/`)
     */
    void connect(std::string_view endpointUrl);

    /**
     * Connect to the selected server with the given username and password.
     * @param endpointUrl Endpoint URL (for example `opc.tcp://localhost:4840/open62541/server/`)
     * @param login       Login credentials with username and password
     */
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

    Node<Client> getNode(const NodeId& id);
    Node<Client> getRootNode();
    Node<Client> getObjectsNode();
    Node<Client> getTypesNode();
    Node<Client> getViewsNode();

    UA_Client* handle() noexcept;
    const UA_Client* handle() const noexcept;

    /// Get client context (for internal use only).
    /// @private
    ClientContext& getContext() noexcept;

private:
    class Connection;
    std::shared_ptr<Connection> connection_;
};

/* ---------------------------------------------------------------------------------------------- */

bool operator==(const Client& left, const Client& right) noexcept;
bool operator!=(const Client& left, const Client& right) noexcept;

}  // namespace opcua
