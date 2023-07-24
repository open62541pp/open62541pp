#include "open62541pp/Server.h"

#include <atomic>
#include <cassert>
#include <mutex>
#include <utility>  // move

#include "open62541pp/Config.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Node.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/helper.h"
#include "open62541pp/services/Attribute.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/Variant.h"

#include "CustomLogger.h"
#include "ServerContext.h"
#include "open62541_impl.h"
#include "version.h"

namespace opcua {

/* ------------------------------------------- Helper ------------------------------------------- */

inline static UA_ServerConfig* getConfig(UA_Server* server) noexcept {
    return UA_Server_getConfig(server);
}

inline static UA_ServerConfig* getConfig(Server* server) noexcept {
    return UA_Server_getConfig(server->handle());
}

/* ----------------------------------------- Connection ----------------------------------------- */

class Server::Connection {
public:
    Connection()
        : server_(UA_Server_new()),
          logger_(getConfig(server_)->logger) {}

    ~Connection() {
        // don't use stop method here because it might throw an exception
        if (running_) {
            UA_Server_run_shutdown(handle());
        }
        UA_Server_delete(handle());
    }

    // prevent copy & move
    Connection(const Connection&) = delete;
    Connection(Connection&&) noexcept = delete;
    Connection& operator=(const Connection&) = delete;
    Connection& operator=(Connection&&) noexcept = delete;

    void runStartup() {
        const auto status = UA_Server_run_startup(handle());
        detail::throwOnBadStatus(status);
        running_ = true;
    }

    uint16_t runIterate() {
        if (!running_) {
            runStartup();
        }
        return UA_Server_run_iterate(handle(), false /* don't wait */);
    }

    void run() {
        if (running_) {
            return;
        }
        runStartup();
        std::lock_guard<std::mutex> lock(mutex_);
        while (running_) {
            // https://github.com/open62541/open62541/blob/master/examples/server_mainloop.c
            UA_Server_run_iterate(handle(), true /* wait for messages in the networklayer */);
        }
    }

    void stop() {
        running_ = false;
        // wait for run loop to complete
        std::lock_guard<std::mutex> lock(mutex_);
        const auto status = UA_Server_run_shutdown(handle());
        detail::throwOnBadStatus(status);
    }

    bool isRunning() const noexcept {
        return running_;
    }

    void setLogger(Logger logger) {
        logger_.setLogger(std::move(logger));
    }

    UA_Server* handle() noexcept {
        return server_;
    }

    ServerContext& getContext() noexcept {
        return context_;
    }

private:
    UA_Server* server_;
    ServerContext context_;
    CustomLogger logger_;
    std::atomic<bool> running_{false};
    std::mutex mutex_;
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

Server::Server(uint16_t port, ByteString certificate)
    : connection_(std::make_shared<Connection>()) {
    const auto status = UA_ServerConfig_setMinimal(
        getConfig(this), port, certificate.empty() ? nullptr : certificate.handle()
    );
    detail::throwOnBadStatus(status);
    applyDefaults(getConfig(this));
}

#ifdef UA_ENABLE_ENCRYPTION
Server::Server(
    uint16_t port,
    const ByteString& certificate,
    const ByteString& privateKey,
    const std::vector<ByteString>& trustList,
    const std::vector<ByteString>& issuerList,
    const std::vector<ByteString>& revocationList
)
    : connection_(std::make_shared<Connection>()) {
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
    detail::throwOnBadStatus(status);
    applyDefaults(getConfig(this));
}
#endif

void Server::setLogger(Logger logger) {
    connection_->setLogger(std::move(logger));
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

void Server::setLogin(const std::vector<Login>& logins, bool allowAnonymous) {
    const size_t number = logins.size();
    auto loginsUa = std::vector<UA_UsernamePasswordLogin>(number);

    for (size_t i = 0; i < number; ++i) {
        loginsUa[i].username = detail::allocUaString(logins[i].username);
        loginsUa[i].password = detail::allocUaString(logins[i].password);
    }

    auto* config = getConfig(this);
#if UAPP_OPEN62541_VER_GE(1, 1)
    if (config->accessControl.clear != nullptr) {
        config->accessControl.clear(&config->accessControl);
    }
#else
    if (config->accessControl.deleteMembers != nullptr) {
        config->accessControl.deleteMembers(&config->accessControl);
    }
#endif

    const auto status = UA_AccessControl_default(
        config,
        allowAnonymous,
#if UAPP_OPEN62541_VER_GE(1, 3)
        nullptr,  // UA_CertificateVerification
#endif
        &config->securityPolicies[config->securityPoliciesSize - 1].policyUri,  // NOLINT
        number,
        loginsUa.data()
    );

    for (size_t i = 0; i < number; ++i) {
        UA_String_clear(&loginsUa[i].username);
        UA_String_clear(&loginsUa[i].password);
    }

    detail::throwOnBadStatus(status);
}

std::vector<std::string> Server::getNamespaceArray() {
    Variant variant;
    services::readValue(*this, {0, UA_NS0ID_SERVER_NAMESPACEARRAY}, variant);
    return variant.getArrayCopy<std::string>();
}

uint16_t Server::registerNamespace(std::string_view uri) {
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
) {
    assert(nodeContext != nullptr && value != nullptr);  // NOLINT
    auto& cb = static_cast<ServerContext::NodeContext*>(nodeContext)->valueCallback.onBeforeRead;
    if (cb) {
        cb(asWrapper<DataValue>(*value));
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
) {
    assert(nodeContext != nullptr && value != nullptr);  // NOLINT
    auto& cb = static_cast<ServerContext::NodeContext*>(nodeContext)->valueCallback.onAfterWrite;
    if (cb) {
        cb(asWrapper<DataValue>(*value));
    }
}

void Server::setVariableNodeValueCallback(const NodeId& id, ValueCallback callback) {
    auto* nodeContext = getContext().getOrCreateNodeContext(id);
    nodeContext->valueCallback = std::move(callback);
    detail::throwOnBadStatus(UA_Server_setNodeContext(handle(), id, nodeContext));

    UA_ValueCallback callbackNative;
    callbackNative.onRead = valueCallbackOnRead;
    callbackNative.onWrite = valueCallbackOnWrite;
    detail::throwOnBadStatus(UA_Server_setVariableNode_valueCallback(handle(), id, callbackNative));
}

#ifdef UA_ENABLE_SUBSCRIPTIONS
Subscription<Server> Server::createSubscription() noexcept {
    return {*this, 0U};
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
    return connection_->isRunning();
}

Node<Server> Server::getNode(const NodeId& id) {
    return {*this, id, true};
}

Node<Server> Server::getRootNode() {
    return {*this, {0, UA_NS0ID_ROOTFOLDER}, false};
}

Node<Server> Server::getObjectsNode() {
    return {*this, {0, UA_NS0ID_OBJECTSFOLDER}, false};
}

Node<Server> Server::getTypesNode() {
    return {*this, {0, UA_NS0ID_TYPESFOLDER}, false};
}

Node<Server> Server::getViewsNode() {
    return {*this, {0, UA_NS0ID_VIEWSFOLDER}, false};
}

UA_Server* Server::handle() noexcept {
    return connection_->handle();
}

const UA_Server* Server::handle() const noexcept {
    return connection_->handle();
}

ServerContext& Server::getContext() noexcept {
    return connection_->getContext();
}

/* ---------------------------------------------------------------------------------------------- */

bool operator==(const Server& left, const Server& right) noexcept {
    return (left.handle() == right.handle());
}

bool operator!=(const Server& left, const Server& right) noexcept {
    return !(left == right);
}

}  // namespace opcua
