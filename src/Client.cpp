#include "open62541pp/Client.h"

#include <atomic>
#include <cstddef>
#include <string>
#include <utility>  // move

#include "open62541pp/AccessControl.h"  // Login
#include "open62541pp/Config.h"
#include "open62541pp/DataType.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Node.h"
#include "open62541pp/TypeConverter.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/open62541.h"
#include "open62541pp/services/Attribute.h"  // readValue
#include "open62541pp/services/Subscription.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/Composed.h"

#include "ClientContext.h"
#include "CustomDataTypes.h"
#include "CustomLogger.h"
#include "open62541_impl.h"

namespace opcua {

/* ------------------------------------------- Helper ------------------------------------------- */

inline static UA_ClientConfig* getConfig(UA_Client* client) noexcept {
    return UA_Client_getConfig(client);
}

inline static UA_ClientConfig* getConfig(Client* client) noexcept {
    return UA_Client_getConfig(client->handle());
}

/* --------------------------------------- State callbacks -------------------------------------- */

// State changes in open62541.
// The initial UA_ClientState from v1.0 was replace by two separate states:
// - UA_SecureChannelState
// - UA_SessionState
//
// | v1.0        | ClientState                  |
// |-------------|------------------------------|
// | Connect     | UA_CLIENTSTATE_CONNECTED     |
// |             | UA_CLIENTSTATE_SECURECHANNEL |
// |             | UA_CLIENTSTATE_SESSION       |
// | Disconnect  | UA_CLIENTSTATE_DISCONNECTED  |
// | Kill server | UA_CLIENTSTATE_DISCONNECTED  |
//
// clang-format off
// | ≥ v1.1      | ChannelState                       | SessionState                       | ConnectStatus |
// |-------------|------------------------------------|------------------------------------|---------------|
// | Connect     | UA_SECURECHANNELSTATE_HEL_SENT     | UA_SESSIONSTATE_CLOSED             | 0             |
// |             | UA_SECURECHANNELSTATE_ACK_RECEIVED | UA_SESSIONSTATE_CLOSED             | 0             |
// |             | UA_SECURECHANNELSTATE_OPN_SENT     | UA_SESSIONSTATE_CLOSED             | 0             |
// |             | UA_SECURECHANNELSTATE_OPEN         | UA_SESSIONSTATE_CLOSED             | 0             |
// |             | UA_SECURECHANNELSTATE_CLOSED       | UA_SESSIONSTATE_CLOSED             | 0             |
// |             | UA_SECURECHANNELSTATE_OPEN         | UA_SESSIONSTATE_CREATE_REQUESTED   | 0             |
// |             | UA_SECURECHANNELSTATE_OPEN         | UA_SESSIONSTATE_CREATED            | 0             |
// |             | UA_SECURECHANNELSTATE_OPEN         | UA_SESSIONSTATE_ACTIVATE_REQUESTED | 0             |
// |             | UA_SECURECHANNELSTATE_OPEN         | UA_SESSIONSTATE_ACTIVATED          | 0             |
// | Disconnect  | UA_SECURECHANNELSTATE_OPEN         | UA_SESSIONSTATE_CLOSING            | 0             |
// |             | UA_SECURECHANNELSTATE_CLOSED       | UA_SESSIONSTATE_CLOSED             | 0             |
// | Kill server | UA_SECURECHANNELSTATE_CLOSED       | UA_SESSIONSTATE_CREATED            | 0             |
// |             | UA_SECURECHANNELSTATE_FRESH        | UA_SESSIONSTATE_CREATED            | 0             |
// |             | UA_SECURECHANNELSTATE_FRESH        | UA_SESSIONSTATE_CREATED            | 2158821376    |
// clang-format on

inline static void invokeStateCallback(ClientContext& context, ClientState state) noexcept {
    const auto& callbackArray = context.stateCallbacks;
    const auto& callback = callbackArray.at(static_cast<size_t>(state));
    if (callback) {
        detail::invokeCatchIgnore(callback);
    }
}

#if UAPP_OPEN62541_VER_LE(1, 0)
// state callback for v1.0
static void stateCallback(UA_Client* client, UA_ClientState clientState) noexcept {
    auto& context = getContext(client);
    if (clientState != context.lastClientState) {
        switch (clientState) {
        case UA_CLIENTSTATE_DISCONNECTED:
            invokeStateCallback(context, ClientState::Disconnected);
            break;
        case UA_CLIENTSTATE_CONNECTED:
            invokeStateCallback(context, ClientState::Connected);
            break;
        case UA_CLIENTSTATE_SESSION:
            invokeStateCallback(context, ClientState::SessionActivated);
            break;
        case UA_CLIENTSTATE_SESSION_DISCONNECTED:
            invokeStateCallback(context, ClientState::SessionClosed);
            break;
        default:
            break;
        };
    }
    context.lastClientState = clientState;
}
#else
// state callback for >= v1.1
static void stateCallback(
    UA_Client* client,
    UA_SecureChannelState channelState,
    UA_SessionState sessionState,
    [[maybe_unused]] UA_StatusCode connectStatus
) noexcept {
    auto& context = getContext(client);
    // handle session state first, mainly to handle SessionClosed before Disconnected
    if (sessionState != context.lastSessionState) {
        switch (sessionState) {
        case UA_SESSIONSTATE_ACTIVATED:
            invokeStateCallback(context, ClientState::SessionActivated);
            break;
        case UA_SESSIONSTATE_CLOSED:
            invokeStateCallback(context, ClientState::SessionClosed);
            break;
        default:
            break;
        }
    }
    if (channelState != context.lastChannelState) {
        switch (channelState) {
        case UA_SECURECHANNELSTATE_OPEN:
            invokeStateCallback(context, ClientState::Connected);
            break;
        case UA_SECURECHANNELSTATE_CLOSED:
            invokeStateCallback(context, ClientState::Disconnected);
            break;
        default:
            break;
        }
    }
    context.lastChannelState = channelState;
    context.lastSessionState = sessionState;
}
#endif

/* ----------------------------------------- Connection ----------------------------------------- */

class Client::Connection {
public:
    Connection()
        : client_(UA_Client_new()),
          customDataTypes_(&getConfig(client_)->customDataTypes),
          logger_(getConfig(client_)->logger) {
        applyDefaults();
    }

    ~Connection() {
        UA_Client_disconnect(handle());
        UA_Client_delete(handle());
    }

    // prevent copy & move
    Connection(const Connection&) = delete;
    Connection(Connection&&) noexcept = delete;
    Connection& operator=(const Connection&) = delete;
    Connection& operator=(Connection&&) noexcept = delete;

    void applyDefaults() {
        auto* config = getConfig(handle());
        config->clientContext = &context_;
        config->stateCallback = stateCallback;
    }

    void runIterate(uint16_t timeoutMilliseconds) {
        const auto status = UA_Client_run_iterate(handle(), timeoutMilliseconds);
        detail::throwOnBadStatus(status);
    }

    void run() {
        if (running_) {
            return;
        }
        running_ = true;
        try {
            while (running_) {
                runIterate(1000);
            }
        } catch (...) {
            running_ = false;
            throw;
        }
    }

    void stop() {
        running_ = false;
    }

    bool isRunning() const noexcept {
        return running_;
    }

    UA_Client* handle() noexcept {
        return client_;
    }

    ClientContext& getContext() noexcept {
        return context_;
    }

    auto& getCustomDataTypes() noexcept {
        return customDataTypes_;
    }

    auto& getCustomLogger() noexcept {
        return logger_;
    }

private:
    UA_Client* client_;
    ClientContext context_;
    CustomDataTypes customDataTypes_;
    CustomLogger logger_;
    std::atomic<bool> running_{false};
};

/* ------------------------------------------- Client ------------------------------------------- */

Client::Client()
    : connection_(std::make_shared<Connection>()) {
    const auto status = UA_ClientConfig_setDefault(getConfig(this));
    detail::throwOnBadStatus(status);
    getConfig(this)->securityMode = UA_MESSAGESECURITYMODE_NONE;
    connection_->applyDefaults();
}

#ifdef UA_ENABLE_ENCRYPTION
Client::Client(
    const ByteString& certificate,
    const ByteString& privateKey,
    Span<const ByteString> trustList,
    Span<const ByteString> revocationList
)
    : connection_(std::make_shared<Connection>()) {
    const auto status = UA_ClientConfig_setDefaultEncryption(
        getConfig(this),
        certificate,
        privateKey,
        asNative(trustList.data()),
        trustList.size(),
        asNative(revocationList.data()),
        revocationList.size()
    );
    detail::throwOnBadStatus(status);
    getConfig(this)->securityMode = UA_MESSAGESECURITYMODE_SIGNANDENCRYPT;
    connection_->applyDefaults();
}
#endif

std::vector<ApplicationDescription> Client::findServers(std::string_view serverUrl) {
    UA_ApplicationDescription* array = nullptr;
    size_t arraySize = 0;
    const auto status = UA_Client_findServers(
        handle(),
        std::string(serverUrl).c_str(),  // serverUrl
        0,  // serverUrisSize
        nullptr,  // serverUris
        0,  // localeIdsSize
        nullptr,  // localeIds
        &arraySize,  // registeredServersSize
        &array  // registeredServers
    );
    auto result = detail::fromNativeArray<ApplicationDescription>(array, arraySize);
    UA_Array_delete(array, arraySize, &UA_TYPES[UA_TYPES_APPLICATIONDESCRIPTION]);
    detail::throwOnBadStatus(status);
    return result;
}

std::vector<EndpointDescription> Client::getEndpoints(std::string_view serverUrl) {
    UA_EndpointDescription* array = nullptr;
    size_t arraySize = 0;
    const auto status = UA_Client_getEndpoints(
        handle(),
        std::string(serverUrl).c_str(),  // serverUrl
        &arraySize,  // endpointDescriptionsSize,
        &array  // endpointDescriptions
    );
    auto result = detail::fromNativeArray<EndpointDescription>(array, arraySize);
    UA_Array_delete(array, arraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
    detail::throwOnBadStatus(status);
    return result;
}

void Client::setLogger(Logger logger) {
    connection_->getCustomLogger().setLogger(std::move(logger));
}

void Client::setTimeout(uint32_t milliseconds) {
    getConfig(this)->timeout = milliseconds;
}

void Client::setSecurityMode(MessageSecurityMode mode) {
    getConfig(this)->securityMode = static_cast<UA_MessageSecurityMode>(mode);
}

void Client::setCustomDataTypes(std::vector<DataType> dataTypes) {
    connection_->getCustomDataTypes().setCustomDataTypes(std::move(dataTypes));
}

static void setStateCallback(ClientContext& context, ClientState state, StateCallback&& callback) {
    context.stateCallbacks.at(static_cast<size_t>(state)) = std::move(callback);
}

void Client::onConnected(StateCallback callback) {
    setStateCallback(getContext(), ClientState::Connected, std::move(callback));
}

void Client::onDisconnected(StateCallback callback) {
    setStateCallback(getContext(), ClientState::Disconnected, std::move(callback));
}

void Client::onSessionActivated(StateCallback callback) {
    setStateCallback(getContext(), ClientState::SessionActivated, std::move(callback));
}

void Client::onSessionClosed(StateCallback callback) {
    setStateCallback(getContext(), ClientState::SessionClosed, std::move(callback));
}

void Client::connect(std::string_view endpointUrl) {
    const auto status = UA_Client_connect(handle(), std::string(endpointUrl).c_str());
    detail::throwOnBadStatus(status);
}

void Client::connect(std::string_view endpointUrl, const Login& login) {
#if UAPP_OPEN62541_VER_LE(1, 0)
    const auto func = UA_Client_connect_username;
#else
    const auto func = UA_Client_connectUsername;
#endif
    const auto status = func(
        handle(), std::string(endpointUrl).c_str(), login.username.c_str(), login.password.c_str()
    );
    detail::throwOnBadStatus(status);
}

void Client::disconnect() noexcept {
    UA_Client_disconnect(handle());
}

bool Client::isConnected() noexcept {
#if UAPP_OPEN62541_VER_LE(1, 0)
    return (UA_Client_getState(handle()) >= UA_CLIENTSTATE_CONNECTED);
#else
    UA_SecureChannelState channelState{};
    UA_Client_getState(handle(), &channelState, nullptr, nullptr);
    return (channelState == UA_SECURECHANNELSTATE_OPEN);
#endif
}

std::vector<std::string> Client::getNamespaceArray() {
    return services::readValue(*this, {0, UA_NS0ID_SERVER_NAMESPACEARRAY})
        .getArrayCopy<std::string>();
}

#ifdef UA_ENABLE_SUBSCRIPTIONS
Subscription<Client> Client::createSubscription() {
    SubscriptionParameters parameters{};
    return createSubscription(parameters);
}

Subscription<Client> Client::createSubscription(SubscriptionParameters& parameters) {
    const uint32_t subscriptionId = services::createSubscription(*this, parameters, true);
    return {*this, subscriptionId};
}

std::vector<Subscription<Client>> Client::getSubscriptions() {
    const auto& subscriptions = getContext().subscriptions;
    std::vector<Subscription<Client>> result;
    result.reserve(subscriptions.size());
    for (const auto& [subId, _] : subscriptions) {
        result.emplace_back(*this, subId);
    }
    return result;
}
#endif

void Client::runIterate(uint16_t timeoutMilliseconds) {
    connection_->runIterate(timeoutMilliseconds);
}

void Client::run() {
    connection_->run();
}

void Client::stop() {
    connection_->stop();
}

bool Client::isRunning() const noexcept {
    return connection_->isRunning();
}

Node<Client> Client::getNode(const NodeId& id) {
    return {*this, id};
}

Node<Client> Client::getRootNode() {
    return {*this, {0, UA_NS0ID_ROOTFOLDER}};
}

Node<Client> Client::getObjectsNode() {
    return {*this, {0, UA_NS0ID_OBJECTSFOLDER}};
}

Node<Client> Client::getTypesNode() {
    return {*this, {0, UA_NS0ID_TYPESFOLDER}};
}

Node<Client> Client::getViewsNode() {
    return {*this, {0, UA_NS0ID_VIEWSFOLDER}};
}

UA_Client* Client::handle() noexcept {
    return connection_->handle();
}

const UA_Client* Client::handle() const noexcept {
    return connection_->handle();
}

ClientContext& Client::getContext() noexcept {
    return connection_->getContext();
}

/* ---------------------------------------------------------------------------------------------- */

bool operator==(const Client& lhs, const Client& rhs) noexcept {
    return (lhs.handle() == rhs.handle());
}

bool operator!=(const Client& lhs, const Client& rhs) noexcept {
    return !(lhs == rhs);
}

}  // namespace opcua
