#include <cassert>
#include <chrono>
#include <cstdio>
#include <thread>
#include <utility>  // move

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Helper.h"
#include "open62541pp/Node.h"
#include "open62541pp/NodeId.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"

#include "open62541_impl.h"
#include "version.h"

namespace opcua {

/* ----------------------------------------- Connection ----------------------------------------- */

class Server::Connection {
public:
    Connection() : server_(UA_Server_new()) {}

    ~Connection() {
        stop();
        UA_Server_delete(server_);
    }

    // prevent copy & move
    Connection(const Connection&) = delete;
    Connection(Connection&&) noexcept = delete;
    Connection& operator=(const Connection&) = delete;
    Connection& operator=(Connection&&) noexcept = delete;

    void applyDefaults() {
        auto* config = getConfig();
        config->publishingIntervalLimits.min = 10; // ms
        config->samplingIntervalLimits.min = 10; // ms
#if UAPP_OPEN62541_VER_GE(1, 2)
        config->allowEmptyVariables = UA_RULEHANDLING_ACCEPT;  // allow empty variables
#endif
    }

    void run() {
        if (running_.load()) {
            throw Exception("OPC UA Server already running");
        }

        const auto status = UA_Server_run_startup(server_);
        checkStatusCodeException(status);

        running_.store(true);
        while (this->running_.load()) {
            // references: 
            // https://open62541.org/doc/current/server.html#server-lifecycle
            // https://github.com/open62541/open62541/blob/master/examples/server_mainloop.c
            const auto waitInterval = UA_Server_run_iterate(this->server_, true);
            std::this_thread::sleep_for(std::chrono::milliseconds(waitInterval));
        }
    }

    void stop() {
        if (!running_.load()) return;

        running_.store(false);
        const auto status = UA_Server_run_shutdown(this->server_);
        checkStatusCodeException(status);
    }

    bool isRunning() const { return running_.load(); }

    void setLogger(Logger logger) {
        logger_ = std::move(logger);
        auto* config = getConfig();
        config->logger.log     = log;
        config->logger.context = this;
        config->logger.clear   = nullptr;
    }

    static void log(
        void* context, UA_LogLevel level, UA_LogCategory category, const char* msg, va_list args
    ) {
        assert(context != nullptr);  // NOLINT

        const auto* instance = static_cast<Connection*>(context);
        assert(instance->logger_ != nullptr);  // NOLINT

        // convert printf format + args to string_view
        va_list tmp;
        va_copy(tmp, args);  // NOLINT
        const int bufferSize = std::vsnprintf(nullptr, 0, msg, tmp);  // NOLINT
        va_end(tmp);  // NOLINT
        std::vector<char> buffer(bufferSize + 1);
        std::vsnprintf(buffer.data(), buffer.size(), msg, args);
        const std::string_view sv(buffer.data(), buffer.size());

        instance->logger_(static_cast<LogLevel>(level), static_cast<LogCategory>(category), sv);
    }

    UA_ServerConfig* getConfig() { return UA_Server_getConfig(server_); }
    UA_Server*       handle()    { return server_; }

private:
    UA_Server*        server_;
    std::atomic<bool> running_ {false};
    Logger            logger_{nullptr};
};

/* ------------------------------------------- Server ------------------------------------------- */

Server::Server()
    : connection_(std::make_shared<Connection>()) {
    const auto status = UA_ServerConfig_setDefault(getConfig());
    checkStatusCodeException(status);
    connection_->applyDefaults();
}

Server::Server(uint16_t port)
    : connection_(std::make_shared<Connection>()) {
    const auto status = UA_ServerConfig_setMinimal(getConfig(), port, nullptr);
    checkStatusCodeException(status);
    connection_->applyDefaults();
}

Server::Server(uint16_t port, std::string_view certificate)
    : connection_(std::make_shared<Connection>()) {
    const auto status = UA_ServerConfig_setMinimal(
        getConfig(), port, ByteString(certificate).handle()
    );
    checkStatusCodeException(status);
    connection_->applyDefaults();
}

void Server::setLogger(Logger logger) {
    connection_->setLogger(std::move(logger));
}

// copy to endpoints needed, see: https://github.com/open62541/open62541/issues/1175
static void copyApplicationDescriptionToEndpoints(UA_ServerConfig* config) {
    for (size_t i = 0; i < config->endpointsSize; ++i) {
        auto& refApplicationName = config->endpoints[i].server.applicationName; // NOLINT
        auto& refApplicationUri  = config->endpoints[i].server.applicationUri;  // NOLINT
        auto& refProductUri      = config->endpoints[i].server.productUri;      // NOLINT

        UA_LocalizedText_clear(&refApplicationName);
        UA_String_clear(&refApplicationUri);
        UA_String_clear(&refProductUri);

        UA_LocalizedText_copy(&config->applicationDescription.applicationName, &refApplicationName);
        UA_String_copy(&config->applicationDescription.applicationUri,         &refApplicationUri);
        UA_String_copy(&config->applicationDescription.productUri,             &refProductUri);
    }
}

void Server::setCustomHostname(std::string_view hostname) {
    auto& ref = getConfig()->customHostname;
    UA_String_clear(&ref);
    ref = allocUaString(hostname);
}

void Server::setApplicationName(std::string_view name) {
    auto& ref = getConfig()->applicationDescription.applicationName;
    UA_LocalizedText_clear(&ref);
    ref = UA_LOCALIZEDTEXT_ALLOC("", name.data());
    copyApplicationDescriptionToEndpoints(getConfig());
}

void Server::setApplicationUri(std::string_view uri) {
    auto& ref = getConfig()->applicationDescription.applicationUri;
    UA_String_clear(&ref);
    ref = allocUaString(uri);
    copyApplicationDescriptionToEndpoints(getConfig());
}

void Server::setProductUri(std::string_view uri) {
    auto& ref = getConfig()->applicationDescription.productUri;
    UA_String_clear(&ref);
    ref = allocUaString(uri);
    copyApplicationDescriptionToEndpoints(getConfig());
}

void Server::setLogin(const std::vector<Login>& logins, bool allowAnonymous) {
    size_t number   = logins.size();
    auto   loginsUa = std::vector<UA_UsernamePasswordLogin>(number);

    for (size_t i = 0; i < number; ++i) {
        loginsUa[i].username = allocUaString(logins[i].username);
        loginsUa[i].password = allocUaString(logins[i].password);
    }

    auto* config = getConfig();
#if UAPP_OPEN62541_VER_GE(1, 1)
    if (config->accessControl.clear != nullptr)
        config->accessControl.clear(&config->accessControl);
#else
    if (config->accessControl.deleteMembers != nullptr)
        config->accessControl.deleteMembers(&config->accessControl);
#endif

    const auto status = UA_AccessControl_default(
        config,
        allowAnonymous,
#if UAPP_OPEN62541_VER_GE(1, 3)
        nullptr,  // UA_CertificateVerification
#endif
        &config->securityPolicies[config->securityPoliciesSize-1].policyUri, // NOLINT
        number,
        loginsUa.data()
    );

    for (size_t i = 0; i < number; ++i) {
        UA_String_clear(&loginsUa[i].username);
        UA_String_clear(&loginsUa[i].password);
    }

    checkStatusCodeException(status);
}

void Server::run()             { connection_->run(); }
void Server::stop()            { connection_->stop(); }
bool Server::isRunning() const { return connection_->isRunning(); }

uint16_t Server::registerNamespace(std::string_view name) {
    return UA_Server_addNamespace(handle(), name.data());
}

Node Server::getNode(const NodeId& id) { return Node(*this, id); }
Node Server::getRootNode()             { return Node(*this, UA_NS0ID_ROOTFOLDER); }
Node Server::getObjectsNode()          { return Node(*this, UA_NS0ID_OBJECTSFOLDER); }
Node Server::getTypesNode()            { return Node(*this, UA_NS0ID_TYPESFOLDER); }
Node Server::getViewsNode()            { return Node(*this, UA_NS0ID_VIEWSFOLDER); }
Node Server::getObjectTypesNode()      { return Node(*this, UA_NS0ID_OBJECTTYPESFOLDER); }
Node Server::getVariableTypesNode()    { return Node(*this, UA_NS0ID_VARIABLETYPESFOLDER); }
Node Server::getDataTypesNode()        { return Node(*this, UA_NS0ID_DATATYPESFOLDER); }
Node Server::getReferenceTypesNode()   { return Node(*this, UA_NS0ID_REFERENCETYPESFOLDER); }

UA_Server*       Server::handle()       { return connection_->handle(); }
const UA_Server* Server::handle() const { return connection_->handle(); }

UA_ServerConfig*       Server::getConfig()       { return connection_->getConfig(); }
const UA_ServerConfig* Server::getConfig() const { return connection_->getConfig(); }

} // namespace opcua
