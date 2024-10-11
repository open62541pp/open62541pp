#include "open62541pp/server.hpp"

#include <atomic>
#include <cassert>
#include <mutex>
#include <utility>  // move

#include "open62541pp/datatype.hpp"
#include "open62541pp/detail/connection.hpp"
#include "open62541pp/detail/result_utils.hpp"  // tryInvoke
#include "open62541pp/detail/server_context.hpp"
#include "open62541pp/detail/traits.hpp"  // Overload
#include "open62541pp/event.hpp"
#include "open62541pp/exception.hpp"
#include "open62541pp/node.hpp"
#include "open62541pp/plugin/accesscontrol.hpp"
#include "open62541pp/plugin/nodestore.hpp"
#include "open62541pp/services/attribute_highlevel.hpp"
#include "open62541pp/session.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/types_composed.hpp"
#include "open62541pp/wrapper.hpp"  // asWrapper

namespace opcua {

/* ---------------------------------------- ServerConfig ---------------------------------------- */

ServerConfig::ServerConfig(uint16_t port, const ByteString& certificate)
    : ServerConfig(UA_ServerConfig{}) {
    throwIfBad(UA_ServerConfig_setMinimal(
        handle(), port, certificate.empty() ? nullptr : certificate.handle()
    ));
}

#ifdef UA_ENABLE_ENCRYPTION
ServerConfig::ServerConfig(
    uint16_t port,
    const ByteString& certificate,
    const ByteString& privateKey,
    Span<const ByteString> trustList,
    Span<const ByteString> issuerList,
    Span<const ByteString> revocationList
)
    : ServerConfig(UA_ServerConfig{}) {
    throwIfBad(UA_ServerConfig_setDefaultWithSecurityPolicies(
        handle(),
        port,
        certificate.handle(),
        privateKey.handle(),
        asNative(trustList.data()),
        trustList.size(),
        asNative(issuerList.data()),
        issuerList.size(),
        asNative(revocationList.data()),
        revocationList.size()
    ));
}
#endif

ServerConfig::ServerConfig(UA_ServerConfig& config, detail::ServerContext& context)
    : config_(&config),
      context_(&context) {}

// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
ServerConfig::ServerConfig(UA_ServerConfig&& config)
    : config_(std::exchange(config, {})),
      context_(std::make_unique<detail::ServerContext>()) {}

ServerConfig::~ServerConfig() {
    if (std::holds_alternative<UA_ServerConfig>(config_)) {
        UA_ServerConfig_clean(handle());
    }
}

UA_ServerConfig* ServerConfig::operator->() noexcept {
    return handle();
}

const UA_ServerConfig* ServerConfig::operator->() const noexcept {
    return handle();
}

void ServerConfig::setLogger(LogFunction logger) {
    if (logger) {
        context().logger = std::make_unique<LoggerDefault>(std::move(logger));
#if UAPP_OPEN62541_VER_GE(1, 4)
        context().logger->assign(handle()->logging);
#else
        context().logger->assign(handle()->logger);
#endif
    }
}

void ServerConfig::setCustomDataTypes(std::vector<DataType> types) {
    context().types = std::move(types);
    context().customDataTypes = std::make_unique<UA_DataTypeArray>(
        detail::createDataTypeArray(context().types)
    );
    handle()->customDataTypes = context().customDataTypes.get();
}

static void setHighestSecurityPolicyForUserTokenTransfer(UA_ServerConfig* config) {
    auto& ac = config->accessControl;
    // NOLINTNEXTLINE(misc-const-correctness)
    Span securityPolicies(config->securityPolicies, config->securityPoliciesSize);
    Span userTokenPolicies(
        asWrapper<UserTokenPolicy>(ac.userTokenPolicies), ac.userTokenPoliciesSize
    );
    if (!securityPolicies.empty()) {
        const auto& highestSecurityPoliciyUri = securityPolicies.back().policyUri;
        for (auto& userTokenPolicy : userTokenPolicies) {
            if (userTokenPolicy.getTokenType() != UserTokenType::Anonymous &&
                userTokenPolicy.getSecurityPolicyUri().empty()) {
                userTokenPolicy.getSecurityPolicyUri() = String(highestSecurityPoliciyUri);
            }
        }
    }
}

static void copyUserTokenPoliciesToEndpoints(UA_ServerConfig* config) {
    // copy config->accessControl.userTokenPolicies -> config->endpoints[i].userIdentityTokens
    auto& ac = config->accessControl;
    for (size_t i = 0; i < config->endpointsSize; ++i) {
        auto& endpoint = config->endpoints[i];  // NOLINT
        detail::deallocateArray(
            endpoint.userIdentityTokens,
            endpoint.userIdentityTokensSize,
            UA_TYPES[UA_TYPES_USERTOKENPOLICY]
        );
        endpoint.userIdentityTokens = detail::copyArray(
            ac.userTokenPolicies, ac.userTokenPoliciesSize, UA_TYPES[UA_TYPES_USERTOKENPOLICY]
        );
        endpoint.userIdentityTokensSize = ac.userTokenPoliciesSize;
    }
}

void ServerConfig::setAccessControl(AccessControlBase& accessControl) {
    accessControl.assign(handle()->accessControl);
    setHighestSecurityPolicyForUserTokenTransfer(handle());
    copyUserTokenPoliciesToEndpoints(handle());
}

static ApplicationDescription& getApplicationDescription(UA_ServerConfig* config) noexcept {
    return asWrapper<ApplicationDescription>(config->applicationDescription);
}

// copy to endpoints needed, see: https://github.com/open62541/open62541/issues/1175
static void copyApplicationDescriptionToEndpoints(UA_ServerConfig* config) {
    auto endpoints = Span(asWrapper<EndpointDescription>(config->endpoints), config->endpointsSize);
    for (auto& endpoint : endpoints) {
        endpoint.getServer() = getApplicationDescription(config);
    }
}

void ServerConfig::setApplicationName(std::string_view name) {
    getApplicationDescription(handle()).getApplicationName() = LocalizedText("", name);
    copyApplicationDescriptionToEndpoints(handle());
}

void ServerConfig::setApplicationUri(std::string_view uri) {
    getApplicationDescription(handle()).getApplicationUri() = String(uri);
    copyApplicationDescriptionToEndpoints(handle());
}

void ServerConfig::setProductUri(std::string_view uri) {
    getApplicationDescription(handle()).getProductUri() = String(uri);
    copyApplicationDescriptionToEndpoints(handle());
}

// NOLINTNEXTLINE(bugprone-exception-escape)
UA_ServerConfig* ServerConfig::handle() noexcept {
    return std::visit(
        detail::Overload{
            [](UA_ServerConfig& cfg) { return &cfg; },
            [](UA_ServerConfig* cfg) { return cfg; },
        },
        config_
    );
}

// NOLINTNEXTLINE(bugprone-exception-escape)
const UA_ServerConfig* ServerConfig::handle() const noexcept {
    return std::visit(
        detail::Overload{
            [](const UA_ServerConfig& cfg) { return &cfg; },
            [](const UA_ServerConfig* cfg) { return cfg; },
        },
        config_
    );
}

// NOLINTNEXTLINE(bugprone-exception-escape)
detail::ServerContext& ServerConfig::context() noexcept {
    auto* context = std::visit(
        detail::Overload{
            [](std::unique_ptr<detail::ServerContext>& ctx) { return ctx.get(); },
            [](detail::ServerContext* ctx) { return ctx; },
        },
        context_
    );
    assert(context != nullptr);
    return *context;
}

/* -------------------------------------- ServerConnection -------------------------------------- */

namespace detail {

static UA_StatusCode activateSession(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_EndpointDescription* endpointDescription,
    const UA_ByteString* secureChannelRemoteCertificate,
    const UA_NodeId* sessionId,
    const UA_ExtensionObject* userIdentityToken,
    void** sessionContext
) {
    auto* context = getContext(server);
    if (context == nullptr || context->sessionRegistry.activateSessionUser == nullptr) {
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    // call user-defined function
    auto status = context->sessionRegistry.activateSessionUser(
        server,
        ac,
        endpointDescription,
        secureChannelRemoteCertificate,
        sessionId,
        userIdentityToken,
        sessionContext
    );
    if (isGood(status) && sessionId != nullptr) {
        context->sessionRegistry.sessionIds.insert(asWrapper<NodeId>(*sessionId));
    }
    return status;
}

static void closeSession(
    UA_Server* server, UA_AccessControl* ac, const UA_NodeId* sessionId, void* sessionContext
) {
    auto* context = getContext(server);
    if (context == nullptr || context->sessionRegistry.closeSessionUser == nullptr) {
        return;
    }
    // call user-defined function
    context->sessionRegistry.closeSessionUser(server, ac, sessionId, sessionContext);
    if (sessionId != nullptr) {
        context->sessionRegistry.sessionIds.erase(asWrapper<NodeId>(*sessionId));
    }
}

struct ServerConnection : public ConnectionBase<Server> {
    ServerConnection()
        : server(allocateServer()),
          context(std::make_unique<detail::ServerContext>()),
          config(*detail::getConfig(server), *context) {
        applyDefaults();
        applySessionRegistry();
    }

    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    explicit ServerConnection(ServerConfig&& cfg)
        : server(allocateServer(cfg)),
          context(moveContext(cfg)),
          config(*detail::getConfig(server), *context) {
        applyDefaults();
        applySessionRegistry();
    }

    static UA_Server* allocateServer() {
        auto* server = UA_Server_new();
        if (server == nullptr) {
            throw BadStatus(UA_STATUSCODE_BADOUTOFMEMORY);
        }
        return server;
    }

    static UA_Server* allocateServer(ServerConfig& config) {
        auto* server = UA_Server_newWithConfig(config.handle());  // shallow copy
        if (server == nullptr) {
            throw BadStatus(UA_STATUSCODE_BADOUTOFMEMORY);
        }
        config.config_ = {};  // empty config
        return server;
    }

    static std::unique_ptr<detail::ServerContext> moveContext(ServerConfig& config) {
        std::unique_ptr<detail::ServerContext> context;
        context.swap(std::get<std::unique_ptr<detail::ServerContext>>(config.context_));
        return context;
    }

    ~ServerConnection() {
        // don't use stop method here because it might throw an exception
        if (running) {
            UA_Server_run_shutdown(server);
        }
        UA_Server_delete(server);
    }

    ServerConnection(const ServerConnection&) = delete;
    ServerConnection(ServerConnection&&) noexcept = delete;
    ServerConnection& operator=(const ServerConnection&) = delete;
    ServerConnection& operator=(ServerConnection&&) noexcept = delete;

    void applySessionRegistry() {
        // Make sure to call this function only once after access control is initialized or changed.
        // The function pointers to activateSession / closeSession might not be unique and the
        // the pointer comparison might fail resulting in stack overflows:
        // - https://github.com/open62541pp/open62541pp/issues/285
        // - https://stackoverflow.com/questions/31209693/static-library-linked-two-times
        if (config->accessControl.activateSession != &activateSession) {
            context->sessionRegistry.activateSessionUser = config->accessControl.activateSession;
            config->accessControl.activateSession = &activateSession;
        }
        if (config->accessControl.closeSession != &closeSession) {
            context->sessionRegistry.closeSessionUser = config->accessControl.closeSession;
            config->accessControl.closeSession = &closeSession;
        }
    }

    void applyDefaults() {
#if UAPP_OPEN62541_VER_GE(1, 3)
        config->context = this;
#else
        const auto status = UA_Server_setNodeContext(
            server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), this
        );
        assert(status == UA_STATUSCODE_GOOD);
#endif
#ifdef UA_ENABLE_SUBSCRIPTIONS
        config->publishingIntervalLimits.min = 10;  // ms
        config->samplingIntervalLimits.min = 10;  // ms
#endif
#if UAPP_OPEN62541_VER_GE(1, 2)
        config->allowEmptyVariables = UA_RULEHANDLING_ACCEPT;  // allow empty variables
#endif
    }

    void runStartup() {
        applyDefaults();
        throwIfBad(UA_Server_run_startup(server));
        running = true;
    }

    uint16_t runIterate() {
        if (!running) {
            runStartup();
        }
        auto interval = UA_Server_run_iterate(server, false /* don't wait */);
        context->exceptionCatcher.rethrow();
        return interval;
    }

    void run() {
        if (running) {
            return;
        }
        runStartup();
        const std::lock_guard lock(mutexRun);
        try {
            while (running) {
                // https://github.com/open62541/open62541/blob/master/examples/server_mainloop.c
                UA_Server_run_iterate(server, true /* wait for messages in the networklayer */);
                context->exceptionCatcher.rethrow();
            }
        } catch (...) {
            running = false;
            throw;
        }
    }

    void stop() {
        running = false;
        // wait for run loop to complete
        const std::lock_guard<std::mutex> lock(mutexRun);
        throwIfBad(UA_Server_run_shutdown(server));
    }

    UA_Server* server;
    std::unique_ptr<detail::ServerContext> context;
    ServerConfig config;
    std::atomic<bool> running{false};
    std::mutex mutexRun;
};

}  // namespace detail

/* ------------------------------------------- Server ------------------------------------------- */

Server::Server()
    : connection_(std::make_unique<detail::ServerConnection>(ServerConfig())) {}

Server::Server(ServerConfig&& config)
    : connection_(std::make_unique<detail::ServerConnection>(std::move(config))) {}

Server::Server(uint16_t port, const ByteString& certificate)
    : connection_(std::make_unique<detail::ServerConnection>(ServerConfig(port, certificate))) {}

#ifdef UA_ENABLE_ENCRYPTION
Server::Server(
    uint16_t port,
    const ByteString& certificate,
    const ByteString& privateKey,
    Span<const ByteString> trustList,
    Span<const ByteString> issuerList,
    Span<const ByteString> revocationList
)
    : connection_(std::make_unique<detail::ServerConnection>(
          ServerConfig(port, certificate, privateKey, trustList, issuerList, revocationList)
      )) {}
#endif

Server::~Server() = default;

void Server::setLogger(LogFunction logger) {
    connection_->config.setLogger(std::move(logger));
}

void Server::setCustomHostname([[maybe_unused]] std::string_view hostname) {
#if UAPP_OPEN62541_VER_LE(1, 3)
    asWrapper<String>(detail::getConfig(*this).customHostname) = String(hostname);
#endif
}

void Server::setApplicationName(std::string_view name) {
    connection_->config.setApplicationName(name);
}

void Server::setApplicationUri(std::string_view uri) {
    connection_->config.setApplicationUri(uri);
}

void Server::setProductUri(std::string_view uri) {
    connection_->config.setProductUri(uri);
}

void Server::setCustomDataTypes(std::vector<DataType> dataTypes) {
    connection_->config.setCustomDataTypes(std::move(dataTypes));
}

void Server::setAccessControl(AccessControlBase& accessControl) {
    connection_->config.setAccessControl(accessControl);
    connection_->applySessionRegistry();
}

void Server::setAccessControl(std::unique_ptr<AccessControlBase> accessControl) {
    if (accessControl) {
        connection_->context->accessControl = std::move(accessControl);
        connection_->config.setAccessControl(*connection_->context->accessControl);
        connection_->applySessionRegistry();
    }
}

std::vector<Session> Server::getSessions() {
    std::vector<Session> sessions;
    for (auto&& id : connection_->context->sessionRegistry.sessionIds) {
        sessions.emplace_back(*this, id);
    }
    return sessions;
}

std::vector<std::string> Server::getNamespaceArray() {
    return services::readValue(*this, {0, UA_NS0ID_SERVER_NAMESPACEARRAY})
        .value()
        .getArrayCopy<std::string>();
}

NamespaceIndex Server::registerNamespace(std::string_view uri) {
    return UA_Server_addNamespace(handle(), std::string(uri).c_str());
}

static void valueCallbackOnRead(
    [[maybe_unused]] UA_Server* server,
    [[maybe_unused]] const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    [[maybe_unused]] const UA_NodeId* nodeId,
    void* nodeContext,
    [[maybe_unused]] const UA_NumericRange* range,
    const UA_DataValue* value
) noexcept {
    assert(nodeContext != nullptr && value != nullptr);
    auto& cb = static_cast<detail::NodeContext*>(nodeContext)->valueCallback.onBeforeRead;
    if (cb) {
        detail::tryInvoke([&] { cb(asWrapper<DataValue>(*value)); });
    }
}

static void valueCallbackOnWrite(
    [[maybe_unused]] UA_Server* server,
    [[maybe_unused]] const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    [[maybe_unused]] const UA_NodeId* nodeId,
    void* nodeContext,
    [[maybe_unused]] const UA_NumericRange* range,
    const UA_DataValue* value
) noexcept {
    assert(nodeContext != nullptr && value != nullptr);
    auto& cb = static_cast<detail::NodeContext*>(nodeContext)->valueCallback.onAfterWrite;
    if (cb) {
        detail::tryInvoke([&] { cb(asWrapper<DataValue>(*value)); });
    }
}

void Server::setVariableNodeValueCallback(const NodeId& id, ValueCallback callback) {
    auto* nodeContext = detail::getContext(*this).nodeContexts[id];
    nodeContext->valueCallback = std::move(callback);
    throwIfBad(UA_Server_setNodeContext(handle(), id, nodeContext));

    UA_ValueCallback callbackNative;
    callbackNative.onRead = valueCallbackOnRead;
    callbackNative.onWrite = valueCallbackOnWrite;
    throwIfBad(UA_Server_setVariableNode_valueCallback(handle(), id, callbackNative));
}

inline static NumericRange asRange(const UA_NumericRange* range) noexcept {
    return range == nullptr ? NumericRange() : NumericRange(*range);
}

static UA_StatusCode valueSourceRead(
    [[maybe_unused]] UA_Server* server,
    [[maybe_unused]] const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    [[maybe_unused]] const UA_NodeId* nodeId,
    void* nodeContext,
    UA_Boolean includeSourceTimestamp,
    const UA_NumericRange* range,
    UA_DataValue* value
) noexcept {
    assert(nodeContext != nullptr && value != nullptr);
    auto& callback = static_cast<detail::NodeContext*>(nodeContext)->dataSource.read;
    if (callback) {
        auto result = detail::tryInvoke(
            callback, asWrapper<DataValue>(*value), asRange(range), includeSourceTimestamp
        );
        return result.code();
    }
    return UA_STATUSCODE_BADINTERNALERROR;
}

static UA_StatusCode valueSourceWrite(
    [[maybe_unused]] UA_Server* server,
    [[maybe_unused]] const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    [[maybe_unused]] const UA_NodeId* nodeId,
    void* nodeContext,
    const UA_NumericRange* range,
    const UA_DataValue* value
) noexcept {
    assert(nodeContext != nullptr && value != nullptr);
    auto& callback = static_cast<detail::NodeContext*>(nodeContext)->dataSource.write;
    if (callback) {
        auto result = detail::tryInvoke(callback, asWrapper<DataValue>(*value), asRange(range));
        return result.code();
    }
    return UA_STATUSCODE_BADINTERNALERROR;
}

void Server::setVariableNodeValueBackend(const NodeId& id, ValueBackendDataSource backend) {
    auto* nodeContext = detail::getContext(*this).nodeContexts[id];
    nodeContext->dataSource = std::move(backend);
    throwIfBad(UA_Server_setNodeContext(handle(), id, nodeContext));

    UA_DataSource dataSourceNative;
    dataSourceNative.read = valueSourceRead;
    dataSourceNative.write = valueSourceWrite;
    throwIfBad(UA_Server_setVariableNode_dataSource(handle(), id, dataSourceNative));
}

#ifdef UA_ENABLE_SUBSCRIPTIONS
Subscription<Server> Server::createSubscription() noexcept {
    return {*this, 0U};
}
#endif

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
Event Server::createEvent(const NodeId& eventType) {
    return Event(*this, eventType);
}
#endif

uint16_t Server::runIterate() {
    return connection_->runIterate();
}

void Server::run() {
    connection_->run();
}

void Server::stop() {
    connection_->stop();
}

bool Server::isRunning() const noexcept {
    return connection_->running;
}

Node<Server> Server::getNode(NodeId id) {
    return {*this, std::move(id)};
}

Node<Server> Server::getRootNode() {
    return {*this, {0, UA_NS0ID_ROOTFOLDER}};
}

Node<Server> Server::getObjectsNode() {
    return {*this, {0, UA_NS0ID_OBJECTSFOLDER}};
}

Node<Server> Server::getTypesNode() {
    return {*this, {0, UA_NS0ID_TYPESFOLDER}};
}

Node<Server> Server::getViewsNode() {
    return {*this, {0, UA_NS0ID_VIEWSFOLDER}};
}

UA_Server* Server::handle() noexcept {
    return connection_->server;
}

const UA_Server* Server::handle() const noexcept {
    return connection_->server;
}

/* -------------------------------------- Helper functions -------------------------------------- */

namespace detail {

UA_ServerConfig* getConfig(UA_Server* server) noexcept {
    return UA_Server_getConfig(server);
}

UA_ServerConfig& getConfig(Server& server) noexcept {
    return *getConfig(server.handle());
}

UA_Logger* getLogger(UA_ServerConfig* config) noexcept {
    if (config == nullptr) {
        return nullptr;
    }
#if UAPP_OPEN62541_VER_GE(1, 4)
    return config->logging;
#else
    return &config->logger;
#endif
}

ServerConnection* getConnection(UA_Server* server) noexcept {
#if UAPP_OPEN62541_VER_GE(1, 3)
    auto* config = getConfig(server);
    if (config == nullptr) {
        return nullptr;
    }
    // UA_ServerConfig.context pointer available since open62541 v1.3
    auto* connection = static_cast<detail::ServerConnection*>(config->context);
#else
    if (server == nullptr) {
        return nullptr;
    }
    // use node context of server object as fallback
    void* nodeContext = nullptr;
    const auto status = UA_Server_getNodeContext(
        server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), &nodeContext
    );
    assert(status == UA_STATUSCODE_GOOD);
    auto* connection = static_cast<detail::ServerConnection*>(nodeContext);
#endif
    assert(connection != nullptr);
    assert(connection->server == server);
    return connection;
}

ServerConnection& getConnection(Server& server) noexcept {
    auto* connection = server.connection_.get();
    assert(connection != nullptr);
    return *connection;
}

Server* getWrapper(UA_Server* server) noexcept {
    auto* connection = getConnection(server);
    if (connection == nullptr) {
        return nullptr;
    }
    return connection->wrapperPtr();
}

ServerContext* getContext(UA_Server* server) noexcept {
    auto* connection = getConnection(server);
    if (connection == nullptr) {
        return nullptr;
    }
    return connection->context.get();
}

ServerContext& getContext(Server& server) noexcept {
    auto* context = getConnection(server).context.get();
    assert(context != nullptr);
    return *context;
}

}  // namespace detail

}  // namespace opcua
