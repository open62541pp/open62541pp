#include "open62541pp/Client.h"

#include <atomic>
#include <cstddef>
#include <iterator>
#include <string>
#include <utility>  // move

#include "open62541pp/AccessControl.h"  // Login
#include "open62541pp/Config.h"
#include "open62541pp/DataType.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Node.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/ClientContext.h"
#include "open62541pp/detail/ConnectionBase.h"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/services/Attribute.h"  // readValue
#include "open62541pp/services/Subscription.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/NodeId.h"

#include "ClientConfig.h"

namespace opcua {

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

inline static void invokeStateCallback(
    detail::ClientContext& context, detail::ClientState state
) noexcept {
    const auto& callback = context.stateCallbacks.at(static_cast<size_t>(state));
    if (callback) {
        context.exceptionCatcher.invoke(callback);
    }
}

#if UAPP_OPEN62541_VER_LE(1, 0)
// state callback for v1.0
static void stateCallback(UA_Client* client, UA_ClientState clientState) noexcept {
    auto* context = detail::getContext(client);
    if (context == nullptr) {
        return;
    }
    if (clientState != context->lastClientState) {
        switch (clientState) {
        case UA_CLIENTSTATE_DISCONNECTED:
            invokeStateCallback(*context, detail::ClientState::Disconnected);
            break;
        case UA_CLIENTSTATE_CONNECTED:
            invokeStateCallback(*context, detail::ClientState::Connected);
            break;
        case UA_CLIENTSTATE_SESSION:
            invokeStateCallback(*context, detail::ClientState::SessionActivated);
            break;
        case UA_CLIENTSTATE_SESSION_DISCONNECTED:
            invokeStateCallback(*context, detail::ClientState::SessionClosed);
            break;
        default:
            break;
        };
    }
    context->lastClientState = clientState;
}
#else
// state callback for >= v1.1
static void stateCallback(
    UA_Client* client,
    UA_SecureChannelState channelState,
    UA_SessionState sessionState,
    [[maybe_unused]] UA_StatusCode connectStatus
) noexcept {
    auto* context = detail::getContext(client);
    if (context == nullptr) {
        return;
    }
    // handle session state first, mainly to handle SessionClosed before Disconnected
    if (sessionState != context->lastSessionState) {
        switch (sessionState) {
        case UA_SESSIONSTATE_ACTIVATED:
            invokeStateCallback(*context, detail::ClientState::SessionActivated);
            break;
        case UA_SESSIONSTATE_CLOSED:
            invokeStateCallback(*context, detail::ClientState::SessionClosed);
            break;
        default:
            break;
        }
    }
    if (channelState != context->lastChannelState) {
        switch (channelState) {
        case UA_SECURECHANNELSTATE_OPEN:
            invokeStateCallback(*context, detail::ClientState::Connected);
            break;
        case UA_SECURECHANNELSTATE_CLOSED:
            invokeStateCallback(*context, detail::ClientState::Disconnected);
            break;
        default:
            break;
        }
    }
    context->lastChannelState = channelState;
    context->lastSessionState = sessionState;
}
#endif

/* -------------------------------------- Connection state -------------------------------------- */

namespace detail {

[[nodiscard]] static UA_Client* allocateClient() {
    auto* client = UA_Client_new();
    if (client == nullptr) {
        throw BadStatus(UA_STATUSCODE_BADOUTOFMEMORY);
    }
    return client;
}

struct ClientConnection : public ConnectionBase<Client> {
    ClientConnection()
        : client(allocateClient()),
          config(*detail::getConfig(client)) {
        applyDefaults();
    }

    ~ClientConnection() {
        UA_Client_disconnect(client);
        UA_Client_delete(client);
    }

    ClientConnection(const ClientConnection&) = delete;
    ClientConnection(ClientConnection&&) noexcept = delete;
    ClientConnection& operator=(const ClientConnection&) = delete;
    ClientConnection& operator=(ClientConnection&&) noexcept = delete;

    void applyDefaults() {
        config->clientContext = this;
        config->stateCallback = stateCallback;
    }

    void runIterate(uint16_t timeoutMilliseconds) {
        const auto status = UA_Client_run_iterate(client, timeoutMilliseconds);
        throwIfBad(status);
        context.exceptionCatcher.rethrow();
    }

    void run() {
        if (running) {
            return;
        }
        running = true;
        try {
            while (running) {
                runIterate(1000);
                context.exceptionCatcher.rethrow();
            }
        } catch (...) {
            running = false;
            throw;
        }
    }

    void stop() {
        running = false;
    }

    UA_Client* client;
    ClientConfig config;
    detail::ClientContext context;
    std::atomic<bool> running{false};
};

}  // namespace detail

/* ------------------------------------------- Client ------------------------------------------- */

Client::Client(Logger logger)
    : connection_(std::make_shared<detail::ClientConnection>()) {
    // The logger should be set as soon as possible, ideally even before UA_ClientConfig_setDefault.
    // However, the logger gets overwritten by UA_ClientConfig_setDefault() in older versions of
    // open62541. The best we can do in this case, is to first call UA_ClientConfig_setDefault and
    // then setLogger.
    auto setConfig = [&] {
        const auto status = UA_ClientConfig_setDefault(detail::getConfig(handle()));
        throwIfBad(status);
    };
#if UAPP_OPEN62541_VER_GE(1, 1)
    setLogger(std::move(logger));
    setConfig();
#else
    setConfig();
    setLogger(std::move(logger));
#endif
    detail::getConfig(*this).securityMode = UA_MESSAGESECURITYMODE_NONE;
    connection_->applyDefaults();
}

#ifdef UA_ENABLE_ENCRYPTION
Client::Client(
    const ByteString& certificate,
    const ByteString& privateKey,
    Span<const ByteString> trustList,
    Span<const ByteString> revocationList
)
    : connection_(std::make_shared<detail::ClientConnection>()) {
    const auto status = UA_ClientConfig_setDefaultEncryption(
        getConfig(handle()),
        certificate,
        privateKey,
        asNative(trustList.data()),
        trustList.size(),
        asNative(revocationList.data()),
        revocationList.size()
    );
    throwIfBad(status);
    getConfig(*this).securityMode = UA_MESSAGESECURITYMODE_SIGNANDENCRYPT;
    connection_->applyDefaults();
}
#endif

std::vector<ApplicationDescription> Client::findServers(std::string_view serverUrl) {
    size_t arraySize{};
    UA_ApplicationDescription* array{};
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
    std::vector<ApplicationDescription> result(
        std::make_move_iterator(array),
        std::make_move_iterator(array + arraySize)  // NOLINT
    );
    UA_Array_delete(array, arraySize, &UA_TYPES[UA_TYPES_APPLICATIONDESCRIPTION]);
    throwIfBad(status);
    return result;
}

std::vector<EndpointDescription> Client::getEndpoints(std::string_view serverUrl) {
    size_t arraySize{};
    UA_EndpointDescription* array{};
    const auto status = UA_Client_getEndpoints(
        handle(),
        std::string(serverUrl).c_str(),  // serverUrl
        &arraySize,  // endpointDescriptionsSize,
        &array  // endpointDescriptions
    );
    std::vector<EndpointDescription> result(
        std::make_move_iterator(array),
        std::make_move_iterator(array + arraySize)  // NOLINT
    );
    UA_Array_delete(array, arraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
    throwIfBad(status);
    return result;
}

void Client::setLogger(Logger logger) {
    connection_->config.setLogger(std::move(logger));
}

void Client::setTimeout(uint32_t milliseconds) {
    detail::getConfig(*this).timeout = milliseconds;
}

void Client::setSecurityMode(MessageSecurityMode mode) {
    detail::getConfig(*this).securityMode = static_cast<UA_MessageSecurityMode>(mode);
}

void Client::setCustomDataTypes(std::vector<DataType> dataTypes) {
    connection_->config.setCustomDataTypes(std::move(dataTypes));
}

static void setStateCallback(Client& client, detail::ClientState state, StateCallback&& callback) {
    detail::getContext(client).stateCallbacks.at(static_cast<size_t>(state)) = std::move(callback);
}

void Client::onConnected(StateCallback callback) {
    setStateCallback(*this, detail::ClientState::Connected, std::move(callback));
}

void Client::onDisconnected(StateCallback callback) {
    setStateCallback(*this, detail::ClientState::Disconnected, std::move(callback));
}

void Client::onSessionActivated(StateCallback callback) {
    setStateCallback(*this, detail::ClientState::SessionActivated, std::move(callback));
}

void Client::onSessionClosed(StateCallback callback) {
    setStateCallback(*this, detail::ClientState::SessionClosed, std::move(callback));
}

void Client::connect(std::string_view endpointUrl) {
    const auto status = UA_Client_connect(handle(), std::string(endpointUrl).c_str());
    throwIfBad(status);
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
    throwIfBad(status);
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
    auto& subscriptions = detail::getContext(*this).subscriptions;
    subscriptions.eraseStale();
    auto lock = subscriptions.acquireLock();
    const auto& map = subscriptions.underlying();
    std::vector<Subscription<Client>> result;
    result.reserve(map.size());
    for (const auto& [subId, _] : map) {
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
    return connection_->running;
}

Node<Client> Client::getNode(NodeId id) {
    return {*this, std::move(id)};
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
    return connection_->client;
}

const UA_Client* Client::handle() const noexcept {
    return connection_->client;
}

/* -------------------------------------- Helper functions -------------------------------------- */

namespace detail {

UA_ClientConfig* getConfig(UA_Client* client) noexcept {
    return UA_Client_getConfig(client);
}

UA_ClientConfig& getConfig(Client& client) noexcept {
    return *getConfig(client.handle());
}

ClientConnection* getConnection(UA_Client* client) noexcept {
    auto* config = getConfig(client);
    if (config == nullptr) {
        return nullptr;
    }
    auto* state = static_cast<detail::ClientConnection*>(config->clientContext);
    assert(state != nullptr);
    assert(state->client == client);
    return state;
}

ClientConnection& getConnection(Client& client) noexcept {
    auto* state = client.connection_.get();
    assert(state != nullptr);
    return *state;
}

Client* getWrapper(UA_Client* client) noexcept {
    auto* state = getConnection(client);
    if (state == nullptr) {
        return nullptr;
    }
    return state->wrapperPtr();
}

ClientContext* getContext(UA_Client* client) noexcept {
    auto* state = getConnection(client);
    if (state == nullptr) {
        return nullptr;
    }
    return &state->context;
}

ClientContext& getContext(Client& client) noexcept {
    return getConnection(client).context;
}

}  // namespace detail

}  // namespace opcua
