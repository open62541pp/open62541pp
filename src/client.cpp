#include "open62541pp/client.hpp"

#include <atomic>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <string>
#include <utility>  // move

#include "open62541pp/config.hpp"
#include "open62541pp/datatype.hpp"
#include "open62541pp/detail/client_context.hpp"
#include "open62541pp/detail/connection.hpp"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/exception.hpp"
#include "open62541pp/node.hpp"
#include "open62541pp/plugin/accesscontrol_default.hpp"  // Login
#include "open62541pp/result.hpp"
#include "open62541pp/services/attribute_highlevel.hpp"  // readValue
#include "open62541pp/services/subscription.hpp"
#include "open62541pp/typewrapper.hpp"

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

inline static void invokeStateCallback(UA_Client* client, detail::ClientState state) noexcept {
    auto* context = detail::getContext(client);
    if (context == nullptr) {
        return;
    }
    const auto stateIndex = static_cast<size_t>(state);
    if (const auto& callback = context->stateCallbacks.at(stateIndex)) {
        context->exceptionCatcher.invoke(callback);
    }
    if (const auto& callback = context->clientStateCallbacks.at(stateIndex)) {
        auto* wrapper = detail::getWrapper(client);
        if (wrapper != nullptr) {
            context->exceptionCatcher.invoke(callback, *wrapper);
        }
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
            invokeStateCallback(client, detail::ClientState::Disconnected);
            break;
        case UA_CLIENTSTATE_CONNECTED:
            invokeStateCallback(client, detail::ClientState::Connected);
            break;
        case UA_CLIENTSTATE_SESSION:
            invokeStateCallback(client, detail::ClientState::SessionActivated);
            break;
        case UA_CLIENTSTATE_SESSION_DISCONNECTED:
            invokeStateCallback(client, detail::ClientState::SessionClosed);
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
            invokeStateCallback(client, detail::ClientState::SessionActivated);
            break;
        case UA_SESSIONSTATE_CLOSED:
            invokeStateCallback(client, detail::ClientState::SessionClosed);
            break;
        default:
            break;
        }
    }
    if (channelState != context->lastChannelState) {
        switch (channelState) {
        case UA_SECURECHANNELSTATE_OPEN:
            invokeStateCallback(client, detail::ClientState::Connected);
            break;
        case UA_SECURECHANNELSTATE_CLOSED:
            invokeStateCallback(client, detail::ClientState::Disconnected);
            break;
        default:
            break;
        }
    }
    context->lastChannelState = channelState;
    context->lastSessionState = sessionState;
}
#endif

/* ---------------------------------------- ClientConfig ---------------------------------------- */

ClientConfig::ClientConfig()
    : ClientConfig(UA_ClientConfig{}) {
    throwIfBad(UA_ClientConfig_setDefault(handle()));
}

#ifdef UA_ENABLE_ENCRYPTION
ClientConfig::ClientConfig(
    const ByteString& certificate,
    const ByteString& privateKey,
    Span<const ByteString> trustList,
    Span<const ByteString> revocationList
)
    : ClientConfig(UA_ClientConfig{}) {
    throwIfBad(UA_ClientConfig_setDefaultEncryption(
        handle(),
        certificate,
        privateKey,
        asNative(trustList.data()),
        trustList.size(),
        asNative(revocationList.data()),
        revocationList.size()
    ));
}
#endif

// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
ClientConfig::ClientConfig(UA_ClientConfig&& config)
    : config_(std::exchange(config, {})),
      context_(std::make_unique<detail::ClientContext>()) {}

ClientConfig::ClientConfig(UA_ClientConfig& config, detail::ClientContext& context)
    : config_(&config),
      context_(&context) {}

ClientConfig::~ClientConfig() {
    if (std::holds_alternative<UA_ClientConfig>(config_)) {
        // create temporary client to free config
        // reset callbacks to avoid notifications
        handle()->stateCallback = nullptr;
        handle()->inactivityCallback = nullptr;
        handle()->subscriptionInactivityCallback = nullptr;
        auto* client = UA_Client_newWithConfig(handle());
        if (client != nullptr) {
            UA_Client_delete(client);
        }
    }
}

void ClientConfig::setLogger(LogFunction logger) {
    if (logger) {
        context().logger = std::make_unique<LoggerDefault>(std::move(logger));
#if UAPP_OPEN62541_VER_GE(1, 4)
        context().logger->assign(handle()->logging);
#else
        context().logger->assign(handle()->logger);
#endif
    }
}

void ClientConfig::setTimeout(uint32_t milliseconds) noexcept {
    handle()->timeout = milliseconds;
}

template <typename T>
static void setUserIdentityTokenHelper(UA_ClientConfig* config, T&& token) {
    auto& wrapper = asWrapper<ExtensionObject>(config->userIdentityToken);
    wrapper = ExtensionObject::fromDecodedCopy(std::forward<T>(token));
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void ClientConfig::setUserIdentityToken(AnonymousIdentityToken token) {
    setUserIdentityTokenHelper(handle(), std::move(token));
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void ClientConfig::setUserIdentityToken(UserNameIdentityToken token) {
    setUserIdentityTokenHelper(handle(), std::move(token));
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void ClientConfig::setUserIdentityToken(X509IdentityToken token) {
    setUserIdentityTokenHelper(handle(), std::move(token));
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void ClientConfig::setUserIdentityToken(IssuedIdentityToken token) {
    setUserIdentityTokenHelper(handle(), std::move(token));
}

void ClientConfig::setSecurityMode(MessageSecurityMode mode) noexcept {
    handle()->securityMode = static_cast<UA_MessageSecurityMode>(mode);
}

void ClientConfig::setCustomDataTypes(std::vector<DataType> types) {
    context().types = std::move(types);
    context().customDataTypes = std::make_unique<UA_DataTypeArray>(
        detail::createDataTypeArray(context().types)
    );
    handle()->customDataTypes = context().customDataTypes.get();
}

inline static void setStateCallback(
    detail::ClientContext& context, detail::ClientState state, StateCallback&& callback
) {
    context.stateCallbacks.at(static_cast<size_t>(state)) = std::move(callback);
}

inline static void setStateCallback(
    detail::ClientContext& context, detail::ClientState state, ClientStateCallback&& callback
) {
    context.clientStateCallbacks.at(static_cast<size_t>(state)) = std::move(callback);
}

void ClientConfig::onConnected(ClientStateCallback callback) {
    setStateCallback(context(), detail::ClientState::Connected, std::move(callback));
}

void ClientConfig::onDisconnected(ClientStateCallback callback) {
    setStateCallback(context(), detail::ClientState::Disconnected, std::move(callback));
}

void ClientConfig::onSessionActivated(ClientStateCallback callback) {
    setStateCallback(context(), detail::ClientState::SessionActivated, std::move(callback));
}

void ClientConfig::onSessionClosed(ClientStateCallback callback) {
    setStateCallback(context(), detail::ClientState::SessionClosed, std::move(callback));
}

static void inactivityCallback(UA_Client* client) noexcept {
    auto* context = detail::getContext(client);
    if (context == nullptr) {
        return;
    }
    if (context->inactivityCallback != nullptr) {
        context->exceptionCatcher.invoke(context->inactivityCallback);
    }
    if (context->clientInactivityCallback != nullptr) {
        auto* wrapper = detail::getWrapper(client);
        if (wrapper != nullptr) {
            context->exceptionCatcher.invoke(context->clientInactivityCallback, *wrapper);
        }
    }
}

void ClientConfig::onInactive(ClientInactivityCallback callback) {
    context().clientInactivityCallback = std::move(callback);
    handle()->inactivityCallback = inactivityCallback;
}

UA_ClientConfig* ClientConfig::operator->() noexcept {
    return handle();
}

const UA_ClientConfig* ClientConfig::operator->() const noexcept {
    return handle();
}

// NOLINTNEXTLINE(bugprone-exception-escape)
UA_ClientConfig* ClientConfig::handle() noexcept {
    return std::visit(
        detail::Overload{
            [](UA_ClientConfig& cfg) { return &cfg; },
            [](UA_ClientConfig* cfg) { return cfg; },
        },
        config_
    );
}

// NOLINTNEXTLINE(bugprone-exception-escape)
const UA_ClientConfig* ClientConfig::handle() const noexcept {
    return std::visit(
        detail::Overload{
            [](const UA_ClientConfig& cfg) { return &cfg; },
            [](const UA_ClientConfig* cfg) { return cfg; },
        },
        config_
    );
}

// NOLINTNEXTLINE(bugprone-exception-escape)
detail::ClientContext& ClientConfig::context() noexcept {
    auto* context = std::visit(
        detail::Overload{
            [](std::unique_ptr<detail::ClientContext>& ctx) { return ctx.get(); },
            [](detail::ClientContext* ctx) { return ctx; },
        },
        context_
    );
    assert(context != nullptr);
    return *context;
}

/* -------------------------------------- ClientConnection -------------------------------------- */

namespace detail {

struct ClientConnection : public ConnectionBase<Client> {
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    explicit ClientConnection(ClientConfig&& cfg)
        : client(allocateClient(cfg)),
          context(moveContext(cfg)),
          config(*detail::getConfig(client), *context) {
        applyDefaults();
    }

    static UA_Client* allocateClient(ClientConfig& config) {
        auto* client = UA_Client_newWithConfig(config.handle());  // shallow copy
        if (client == nullptr) {
            throw BadStatus(UA_STATUSCODE_BADOUTOFMEMORY);
        }
        config.config_ = {};  // empty config
        return client;
    }

    static std::unique_ptr<detail::ClientContext> moveContext(ClientConfig& config) {
        std::unique_ptr<detail::ClientContext> context;
        context.swap(std::get<std::unique_ptr<detail::ClientContext>>(config.context_));
        return context;
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

    // NOLINTNEXTLINE(readability-make-member-function-const), false positive?
    void runIterate(uint16_t timeoutMilliseconds) {
        throwIfBad(UA_Client_run_iterate(client, timeoutMilliseconds));
        context->exceptionCatcher.rethrow();
    }

    void run() {
        if (running) {
            return;
        }
        running = true;
        try {
            while (running) {
                runIterate(1000);
                context->exceptionCatcher.rethrow();
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
    std::unique_ptr<detail::ClientContext> context;
    ClientConfig config;
    std::atomic<bool> running{false};
};

}  // namespace detail

/* ------------------------------------------- Client ------------------------------------------- */

Client::Client()
    : connection_(std::make_unique<detail::ClientConnection>(ClientConfig())) {}

Client::Client(ClientConfig&& config)
    : connection_(std::make_unique<detail::ClientConnection>(std::move(config))) {}

#ifdef UA_ENABLE_ENCRYPTION
Client::Client(
    const ByteString& certificate,
    const ByteString& privateKey,
    Span<const ByteString> trustList,
    Span<const ByteString> revocationList
)
    : connection_(std::make_unique<detail::ClientConnection>(
          ClientConfig(certificate, privateKey, trustList, revocationList)
      )) {}
#endif

Client::~Client() = default;

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

void Client::setLogger(LogFunction logger) {
    connection_->config.setLogger(std::move(logger));
}

void Client::setTimeout(uint32_t milliseconds) {
    connection_->config.setTimeout(milliseconds);
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void Client::setUserIdentityToken(AnonymousIdentityToken token) {
    connection_->config.setUserIdentityToken(std::move(token));
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void Client::setUserIdentityToken(UserNameIdentityToken token) {
    connection_->config.setUserIdentityToken(std::move(token));
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void Client::setUserIdentityToken(X509IdentityToken token) {
    connection_->config.setUserIdentityToken(std::move(token));
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void Client::setUserIdentityToken(IssuedIdentityToken token) {
    connection_->config.setUserIdentityToken(std::move(token));
}

void Client::setSecurityMode(MessageSecurityMode mode) {
    connection_->config.setSecurityMode(mode);
}

void Client::setCustomDataTypes(std::vector<DataType> dataTypes) {
    connection_->config.setCustomDataTypes(std::move(dataTypes));
}

void Client::onConnected(StateCallback callback) {
    setStateCallback(
        detail::getContext(*this), detail::ClientState::Connected, std::move(callback)
    );
}

void Client::onDisconnected(StateCallback callback) {
    setStateCallback(
        detail::getContext(*this), detail::ClientState::Disconnected, std::move(callback)
    );
}

void Client::onSessionActivated(StateCallback callback) {
    setStateCallback(
        detail::getContext(*this), detail::ClientState::SessionActivated, std::move(callback)
    );
}

void Client::onSessionClosed(StateCallback callback) {
    setStateCallback(
        detail::getContext(*this), detail::ClientState::SessionClosed, std::move(callback)
    );
}

void Client::onInactive(InactivityCallback callback) {
    detail::getContext(*this).inactivityCallback = std::move(callback);
    detail::getConfig(*this).inactivityCallback = inactivityCallback;
}

void Client::connect(std::string_view endpointUrl) {
    throwIfBad(UA_Client_connect(handle(), std::string(endpointUrl).c_str()));
}

void Client::connect(std::string_view endpointUrl, const Login& login) {
#if UAPP_OPEN62541_VER_LE(1, 0)
    const auto func = UA_Client_connect_username;
#else
    const auto func = UA_Client_connectUsername;
#endif
    throwIfBad(func(
        handle(), std::string(endpointUrl).c_str(), login.username.c_str(), login.password.c_str()
    ));
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
        .value()
        .getArrayCopy<std::string>();
}

#ifdef UA_ENABLE_SUBSCRIPTIONS
Subscription<Client> Client::createSubscription() {
    SubscriptionParameters parameters{};
    return createSubscription(parameters);
}

Subscription<Client> Client::createSubscription(SubscriptionParameters& parameters) {
    const uint32_t subscriptionId = services::createSubscription(*this, parameters, true).value();
    return {*this, subscriptionId};
}

std::vector<Subscription<Client>> Client::getSubscriptions() {
    std::vector<Subscription<Client>> result;
    auto& subscriptions = detail::getContext(*this).subscriptions;
    subscriptions.eraseStale();
    subscriptions.iterate([&](const auto& pair) { result.emplace_back(*this, pair.first); });
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

UA_Logger* getLogger(UA_ClientConfig* config) noexcept {
    if (config == nullptr) {
        return nullptr;
    }
#if UAPP_OPEN62541_VER_GE(1, 4)
    return config->logging;
#else
    return &config->logger;
#endif
}

ClientConnection* getConnection(UA_Client* client) noexcept {
    auto* config = getConfig(client);
    if (config == nullptr) {
        return nullptr;
    }
    auto* connection = static_cast<detail::ClientConnection*>(config->clientContext);
    assert(connection != nullptr);
    assert(connection->client == client);
    return connection;
}

ClientConnection& getConnection(Client& client) noexcept {
    auto* connection = client.connection_.get();
    assert(connection != nullptr);
    return *connection;
}

Client* getWrapper(UA_Client* client) noexcept {
    auto* connection = getConnection(client);
    if (connection == nullptr) {
        return nullptr;
    }
    return connection->wrapperPtr();
}

ClientContext* getContext(UA_Client* client) noexcept {
    auto* connection = getConnection(client);
    if (connection == nullptr) {
        return nullptr;
    }
    return connection->context.get();
}

ClientContext& getContext(Client& client) noexcept {
    auto* context = getConnection(client).context.get();
    assert(context != nullptr);
    return *context;
}

}  // namespace detail

}  // namespace opcua
