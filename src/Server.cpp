#include "open62541pp/Server.h"

#include <atomic>
#include <cassert>
#include <cstddef>
#include <mutex>
#include <utility>  // move

#include "open62541pp/AccessControl.h"
#include "open62541pp/Config.h"
#include "open62541pp/DataType.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Event.h"
#include "open62541pp/Node.h"
#include "open62541pp/Session.h"
#include "open62541pp/ValueBackend.h"
#include "open62541pp/Wrapper.h"  // asWrapper
#include "open62541pp/detail/Result.h"  // tryInvoke
#include "open62541pp/detail/ServerContext.h"
#include "open62541pp/detail/open62541/server.h"
#include "open62541pp/services/Attribute.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/Variant.h"

#include "ServerConfig.h"

namespace opcua {

/* ------------------------------------------- Helper ------------------------------------------- */

inline static UA_ServerConfig* getConfig(UA_Server* server) noexcept {
    return UA_Server_getConfig(server);
}

inline static UA_ServerConfig* getConfig(Server* server) noexcept {
    return getConfig(server->handle());
}

/* ----------------------------------------- Connection ----------------------------------------- */

struct Server::Connection {
    explicit Connection(Server& parent)
        : server(UA_Server_new()),
          config(*getConfig(server), parent) {}

    ~Connection() {
        // don't use stop method here because it might throw an exception
        if (running) {
            UA_Server_run_shutdown(server);
        }
        UA_Server_delete(server);
    }

    // prevent copy & move
    Connection(const Connection&) = delete;
    Connection(Connection&&) noexcept = delete;
    Connection& operator=(const Connection&) = delete;
    Connection& operator=(Connection&&) noexcept = delete;

    void runStartup() {
        const auto status = UA_Server_run_startup(server);
        throwIfBad(status);
        running = true;
    }

    uint16_t runIterate() {
        if (!running) {
            runStartup();
        }
        auto interval = UA_Server_run_iterate(server, false /* don't wait */);
        context.exceptionCatcher.rethrow();
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
                context.exceptionCatcher.rethrow();
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
        const auto status = UA_Server_run_shutdown(server);
        throwIfBad(status);
    }

    UA_Server* server;
    ServerConfig config;
    detail::ServerContext context;
    std::atomic<bool> running{false};
    std::mutex mutexRun;
};

/* ------------------------------------------- Server ------------------------------------------- */

static void applyDefaults(UA_ServerConfig* config) {
#ifdef UA_ENABLE_SUBSCRIPTIONS
    config->publishingIntervalLimits.min = 10;  // ms
    config->samplingIntervalLimits.min = 10;  // ms
#endif
#if UAPP_OPEN62541_VER_GE(1, 2)
    config->allowEmptyVariables = UA_RULEHANDLING_ACCEPT;  // allow empty variables
#endif
}

Server::Server(uint16_t port, ByteString certificate, Logger logger)
    : connection_(std::make_shared<Connection>(*this)) {
    // The logger should be set as soon as possible, ideally even before UA_ServerConfig_setMinimal.
    // However, the logger gets overwritten by UA_ServerConfig_setMinimal() in older versions of
    // open62541. The best we can do in this case, is to first call UA_ServerConfig_setMinimal and
    // then setLogger.
    auto setConfig = [&] {
        const auto status = UA_ServerConfig_setMinimal(
            getConfig(this), port, certificate.empty() ? nullptr : certificate.handle()
        );
        throwIfBad(status);
    };
#if UAPP_OPEN62541_VER_GE(1, 1)
    setLogger(std::move(logger));
    setConfig();
#else
    setConfig();
    setLogger(std::move(logger));
#endif
    applyDefaults(getConfig(this));
    setAccessControl(std::make_unique<AccessControlDefault>());
}

#ifdef UA_ENABLE_ENCRYPTION
Server::Server(
    uint16_t port,
    const ByteString& certificate,
    const ByteString& privateKey,
    Span<const ByteString> trustList,
    Span<const ByteString> issuerList,
    Span<const ByteString> revocationList
)
    : connection_(std::make_shared<Connection>(*this)) {
    const auto status = UA_ServerConfig_setDefaultWithSecurityPolicies(
        getConfig(this),
        port,
        certificate.handle(),
        privateKey.handle(),
        asNative(trustList.data()),
        trustList.size(),
        asNative(issuerList.data()),
        issuerList.size(),
        asNative(revocationList.data()),
        revocationList.size()
    );
    throwIfBad(status);
    applyDefaults(getConfig(this));
    setAccessControl(std::make_unique<AccessControlDefault>());
}
#endif

void Server::setLogger(Logger logger) {
    connection_->config.setLogger(std::move(logger));
}

// copy to endpoints needed, see: https://github.com/open62541/open62541/issues/1175
static void copyApplicationDescriptionToEndpoints(UA_ServerConfig* config) {
    for (size_t i = 0; i < config->endpointsSize; ++i) {
        auto& ref = asWrapper<ApplicationDescription>(config->endpoints[i].server);  // NOLINT
        ref = ApplicationDescription(config->applicationDescription);
    }
}

void Server::setCustomHostname(std::string_view hostname) {
    auto& ref = asWrapper<String>(getConfig(this)->customHostname);
    ref = String(hostname);
}

void Server::setApplicationName(std::string_view name) {
    auto& ref = asWrapper<LocalizedText>(getConfig(this)->applicationDescription.applicationName);
    ref = LocalizedText("", name);
    copyApplicationDescriptionToEndpoints(getConfig(this));
}

void Server::setApplicationUri(std::string_view uri) {
    auto& ref = asWrapper<String>(getConfig(this)->applicationDescription.applicationUri);
    ref = String(uri);
    copyApplicationDescriptionToEndpoints(getConfig(this));
}

void Server::setProductUri(std::string_view uri) {
    auto& ref = asWrapper<String>(getConfig(this)->applicationDescription.productUri);
    ref = String(uri);
    copyApplicationDescriptionToEndpoints(getConfig(this));
}

void Server::setAccessControl(AccessControlBase& accessControl) {
    connection_->config.setAccessControl(accessControl);
}

void Server::setAccessControl(std::unique_ptr<AccessControlBase> accessControl) {
    connection_->config.setAccessControl(std::move(accessControl));
}

std::vector<Session> Server::getSessions() const {
    return connection_->config.getSessions();
}

std::vector<std::string> Server::getNamespaceArray() {
    return services::readValue(*this, {0, UA_NS0ID_SERVER_NAMESPACEARRAY})
        .getArrayCopy<std::string>();
}

uint16_t Server::registerNamespace(std::string_view uri) {
    return UA_Server_addNamespace(handle(), std::string(uri).c_str());
}

void Server::setCustomDataTypes(std::vector<DataType> dataTypes) {
    connection_->config.setCustomDataTypes(std::move(dataTypes));
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
        return detail::tryInvokeGetStatus(
            callback, asWrapper<DataValue>(*value), asRange(range), includeSourceTimestamp
        );
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
        return detail::tryInvokeGetStatus(callback, asWrapper<DataValue>(*value), asRange(range));
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

/* ---------------------------------------------------------------------------------------------- */

bool operator==(const Server& lhs, const Server& rhs) noexcept {
    return (lhs.handle() == rhs.handle());
}

bool operator!=(const Server& lhs, const Server& rhs) noexcept {
    return !(lhs == rhs);
}

/* ------------------------------------------- Context ------------------------------------------ */

namespace detail {

ServerContext& getContext(Server& server) noexcept {
    return server.connection_->context;
}

}  // namespace detail

}  // namespace opcua
