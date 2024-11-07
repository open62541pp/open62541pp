#include "open62541pp/client.hpp"

#include <cassert>
#include <iterator>
#include <utility>  // move

#include "open62541pp/config.hpp"
#include "open62541pp/datatype.hpp"
#include "open62541pp/detail/client_context.hpp"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/exception.hpp"
#include "open62541pp/node.hpp"
#include "open62541pp/plugin/accesscontrol_default.hpp"  // Login
#include "open62541pp/result.hpp"
#include "open62541pp/services/attribute_highlevel.hpp"  // readValue
#include "open62541pp/services/subscription.hpp"

namespace opcua {

static UA_Client* allocateClient(UA_ClientConfig& config) noexcept {
#if UAPP_OPEN62541_VER_LE(1, 0)
    auto* client = UA_Client_new();
    auto* clientConfig = UA_Client_getConfig(client);
    if (clientConfig != nullptr) {
        detail::clear(clientConfig->logger);
        *clientConfig = config;
    }
#else
    auto* client = UA_Client_newWithConfig(&config);
#endif
    return client;
}

static void deleteClient(UA_Client* client) noexcept {
    if (client == nullptr) {
        return;
    }
#if UAPP_OPEN62541_VER_LE(1, 0)
    // UA_ClientConfig_deleteMembers won't delete the logger in v1.0
    detail::clear(UA_Client_getConfig(client)->logger);
#endif
    UA_Client_delete(client);
}

static void clearConfig(UA_ClientConfig& config) noexcept {
#if UAPP_OPEN62541_VER_GE(1, 4)
    UA_ClientConfig_clear(&config);
#else
    // create temporary client to clear config
    // reset callbacks to avoid notifications
    config.stateCallback = nullptr;
    config.inactivityCallback = nullptr;
    config.subscriptionInactivityCallback = nullptr;
    deleteClient(allocateClient(config));
#endif
}

/* ---------------------------------------- ClientConfig ---------------------------------------- */

ClientConfig::ClientConfig() {
    throwIfBad(UA_ClientConfig_setDefault(handle()));
}

#ifdef UA_ENABLE_ENCRYPTION
ClientConfig::ClientConfig(
    const ByteString& certificate,
    const ByteString& privateKey,
    Span<const ByteString> trustList,
    Span<const ByteString> revocationList
) {
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
ClientConfig::ClientConfig(UA_ClientConfig&& native)
    : Wrapper(std::exchange(native, {})) {}

ClientConfig::~ClientConfig() {
    clearConfig(native());
}

// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
ClientConfig::ClientConfig(ClientConfig&& other) noexcept
    : Wrapper(std::exchange(other.native(), {})) {}

ClientConfig& ClientConfig::operator=(ClientConfig&& other) noexcept {
    if (this != &other) {
        // clearConfig(native());  // TODO
        native() = std::exchange(other.native(), {});
    }
    return *this;
}

void ClientConfig::setLogger(LogFunction func) {
    if (func) {
        auto adapter = std::make_unique<LoggerDefault>(std::move(func));
        auto* logger = detail::getLogger(handle());
        assert(logger != nullptr);
        detail::clear(*logger);
        *logger = adapter.release()->create(true);
    }
}

void ClientConfig::setTimeout(uint32_t milliseconds) noexcept {
    native().timeout = milliseconds;
}

template <typename T>
static void setUserIdentityTokenHelper(UA_ClientConfig& config, const T& token) {
    asWrapper<ExtensionObject>(config.userIdentityToken) = ExtensionObject::fromDecodedCopy(token);
}

void ClientConfig::setUserIdentityToken(const AnonymousIdentityToken& token) {
    setUserIdentityTokenHelper(native(), token);
}

void ClientConfig::setUserIdentityToken(const UserNameIdentityToken& token) {
    setUserIdentityTokenHelper(native(), token);
}

void ClientConfig::setUserIdentityToken(const X509IdentityToken& token) {
    setUserIdentityTokenHelper(native(), token);
}

void ClientConfig::setUserIdentityToken(const IssuedIdentityToken& token) {
    setUserIdentityTokenHelper(native(), token);
}

void ClientConfig::setSecurityMode(MessageSecurityMode mode) noexcept {
    native().securityMode = static_cast<UA_MessageSecurityMode>(mode);
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

static void invokeStateCallback(
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

static void updateLoggerStackPointer([[maybe_unused]] UA_ClientConfig& config) noexcept {
#if UAPP_OPEN62541_VER_LE(1, 2)
    for (auto& policy : Span(config.securityPolicies, config.securityPoliciesSize)) {
        policy.logger = &config.logger;
    }
#endif
}

static void setWrapperAsContextPointer(Client& client) noexcept {
    client.config()->clientContext = &client;
}

Client::Client()
    : Client(ClientConfig()) {}

// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
Client::Client(ClientConfig&& config)
    : context_(std::make_unique<detail::ClientContext>()),
      client_(allocateClient(config)) {
    if (handle() == nullptr) {
        throw BadStatus(UA_STATUSCODE_BADOUTOFMEMORY);
    }
    config = {};
    this->config()->stateCallback = stateCallback;
    updateLoggerStackPointer(this->config());
    setWrapperAsContextPointer(*this);
}

#ifdef UA_ENABLE_ENCRYPTION
Client::Client(
    const ByteString& certificate,
    const ByteString& privateKey,
    Span<const ByteString> trustList,
    Span<const ByteString> revocationList
)
    : Client(ClientConfig(certificate, privateKey, trustList, revocationList)) {}
#endif

Client::~Client() = default;

Client::Client(Client&& other) noexcept
    : context_(std::move(other.context_)),
      client_(std::move(other.client_)) {
    setWrapperAsContextPointer(*this);
}

Client& Client::operator=(Client&& other) noexcept {
    if (this != &other) {
        context_ = std::move(other.context_);
        client_ = std::move(other.client_);
        setWrapperAsContextPointer(*this);
    }
    return *this;
}

ClientConfig& Client::config() noexcept {
    return asWrapper<ClientConfig>(*detail::getConfig(handle()));
}

const ClientConfig& Client::config() const noexcept {
    return const_cast<Client*>(this)->config();  // NOLINT
}

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

void Client::setCustomDataTypes(Span<const DataType> dataTypes) {
    context().dataTypes = {dataTypes.begin(), dataTypes.end()};
    context().dataTypeArray = std::make_unique<UA_DataTypeArray>(
        detail::createDataTypeArray(context().dataTypes)
    );
    config()->customDataTypes = context().dataTypeArray.get();
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

void Client::onInactive(InactivityCallback callback) {
    context().inactivityCallback = std::move(callback);
    config()->inactivityCallback = [](UA_Client* client) noexcept {
        auto* context = detail::getContext(client);
        if (context != nullptr && context->inactivityCallback != nullptr) {
            context->exceptionCatcher.invoke(context->inactivityCallback);
        }
    };
}

void Client::connect(std::string_view endpointUrl) {
    throwIfBad(UA_Client_connect(handle(), std::string(endpointUrl).c_str()));
}

void Client::connectAsync(std::string_view endpointUrl) {
#if UAPP_OPEN62541_VER_GE(1, 1)
    throwIfBad(UA_Client_connectAsync(handle(), std::string(endpointUrl).c_str()));
#else
    throwIfBad(UA_Client_connect_async(handle(), std::string(endpointUrl).c_str(), nullptr, nullptr)
    );
#endif
}

void Client::connect(std::string_view endpointUrl, const Login& login) {
    config().setUserIdentityToken(UserNameIdentityToken(login.username, login.password));
    connect(endpointUrl);
}

void Client::disconnect() {
    throwIfBad(UA_Client_disconnect(handle()));
}

void Client::disconnectAsync() {
#if UAPP_OPEN62541_VER_GE(1, 1)
    throwIfBad(UA_Client_disconnectAsync(handle()));
#else
    throwIfBad(UA_Client_disconnect_async(handle(), nullptr));
#endif
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
    return createSubscription({});
}

Subscription<Client> Client::createSubscription(const SubscriptionParameters& parameters) {
    const auto response = services::createSubscription(*this, parameters, true, {}, {});
    response.getResponseHeader().getServiceResult().throwIfBad();
    return {*this, response.getSubscriptionId()};
}

std::vector<Subscription<Client>> Client::getSubscriptions() {
    std::vector<Subscription<Client>> result;
    auto& subscriptions = context().subscriptions;
    subscriptions.eraseStale();
    subscriptions.iterate([&](const auto& pair) { result.emplace_back(*this, pair.first); });
    return result;
}
#endif

void Client::runIterate(uint16_t timeoutMilliseconds) {
    throwIfBad(UA_Client_run_iterate(handle(), timeoutMilliseconds));
    context().exceptionCatcher.rethrow();
}

void Client::run() {
    if (context().running) {
        return;
    }
    context().running = true;
    try {
        while (context().running) {
            runIterate(1000);
            context().exceptionCatcher.rethrow();
        }
    } catch (...) {
        context().running = false;
        throw;
    }
}

void Client::stop() {
    context().running = false;
}

bool Client::isRunning() const noexcept {
    return context().running;
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
    return client_.get();
}

const UA_Client* Client::handle() const noexcept {
    return client_.get();
}

detail::ClientContext& Client::context() noexcept {
    return *context_;
}

const detail::ClientContext& Client::context() const noexcept {
    return *context_;
}

void Client::Deleter::operator()(UA_Client* client) noexcept {
    if (client != nullptr) {
        UA_Client_disconnect(client);
        deleteClient(client);
    }
}

Client* asWrapper(UA_Client* client) noexcept {
    auto* config = detail::getConfig(client);
    return config == nullptr ? nullptr : static_cast<Client*>(config->clientContext);
}

/* -------------------------------------- Utility functions ------------------------------------- */

namespace detail {

UA_ClientConfig* getConfig(UA_Client* client) noexcept {
    return UA_Client_getConfig(client);
}

UA_Logger* getLogger(UA_ClientConfig* config) noexcept {
#if UAPP_OPEN62541_VER_GE(1, 4)
    return config == nullptr ? nullptr : config->logging;
#else
    return config == nullptr ? nullptr : &config->logger;
#endif
}

ClientContext* getContext(UA_Client* client) noexcept {
    auto* wrapper = asWrapper(client);
    return wrapper == nullptr ? nullptr : &getContext(*wrapper);
}

ClientContext& getContext(Client& client) noexcept {
    return client.context();
}

ExceptionCatcher* getExceptionCatcher(UA_Client* client) noexcept {
    auto* context = getContext(client);
    return context == nullptr ? nullptr : &context->exceptionCatcher;
}

ExceptionCatcher& getExceptionCatcher(Client& client) noexcept {
    return getContext(client).exceptionCatcher;
}

UA_Client* getHandle(Client& client) noexcept {
    return client.handle();
}

}  // namespace detail

}  // namespace opcua
