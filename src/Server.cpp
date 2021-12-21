#include <chrono>
#include <thread>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Helper.h"
#include "open62541pp/Node.h"
#include "open62541pp/NodeId.h"
#include "open62541pp/Server.h"

#include "open62541_impl.h"
#include "version.h"

namespace opcua {

uint16_t Server::registerNamespace(std::string_view name) {
    return UA_Server_addNamespace(handle(), name.data());
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
    auto& ref = connection_->getConfig()->customHostname;
    UA_String_clear(&ref);
    ref = allocUaString(hostname);
}

void Server::setApplicationName(std::string_view name) {
    auto& ref = connection_->getConfig()->applicationDescription.applicationName;
    UA_LocalizedText_clear(&ref);
    ref = UA_LOCALIZEDTEXT_ALLOC("", name.data());
    copyApplicationDescriptionToEndpoints(connection_->getConfig());
}

void Server::setApplicationUri(std::string_view uri) {
    auto& ref = connection_->getConfig()->applicationDescription.applicationUri;
    UA_String_clear(&ref);
    ref = allocUaString(uri);
    copyApplicationDescriptionToEndpoints(connection_->getConfig());
}

void Server::setProductUri(std::string_view uri) {
    auto& ref = connection_->getConfig()->applicationDescription.productUri;
    UA_String_clear(&ref);
    ref = allocUaString(uri);
    copyApplicationDescriptionToEndpoints(connection_->getConfig());
}

void Server::setLogin(const std::vector<Login>& logins, bool allowAnonymous) {
    size_t number   = logins.size();
    auto   loginsUa = std::vector<UA_UsernamePasswordLogin>(number);

    for (size_t i = 0; i < number; ++i) {
        loginsUa[i].username = allocUaString(logins[i].username);
        loginsUa[i].password = allocUaString(logins[i].password);
    }

    auto* config = connection_->getConfig();
#if UAPP_OPEN62541_VER_GE(1, 1)
    if (config->accessControl.clear != nullptr)
        config->accessControl.clear(&config->accessControl);
#else
    if (config->accessControl.deleteMembers != nullptr)
        config->accessControl.deleteMembers(&config->accessControl);
#endif

    auto status = UA_AccessControl_default(
        config,
        allowAnonymous,
        &config->securityPolicies[config->securityPoliciesSize-1].policyUri, // NOLINT
        number,
        loginsUa.data());

    for (size_t i = 0; i < number; ++i) {
        UA_String_clear(&loginsUa[i].username);
        UA_String_clear(&loginsUa[i].password);
    }

    checkStatusCodeException(status);
}

Node       Server::getNode(const NodeId& id) { return Node(*this, id); }
ObjectNode Server::getRootNode()             { return ObjectNode(*this, UA_NS0ID_ROOTFOLDER); }
ObjectNode Server::getObjectsNode()          { return ObjectNode(*this, UA_NS0ID_OBJECTSFOLDER); }
ObjectNode Server::getTypesNode()            { return ObjectNode(*this, UA_NS0ID_TYPESFOLDER); }
ObjectNode Server::getViewsNode()            { return ObjectNode(*this, UA_NS0ID_VIEWSFOLDER); }
ObjectNode Server::getObjectTypesNode()      { return ObjectNode(*this, UA_NS0ID_OBJECTTYPESFOLDER); }
ObjectNode Server::getVariableTypesNode()    { return ObjectNode(*this, UA_NS0ID_VARIABLETYPESFOLDER); }
ObjectNode Server::getDataTypesNode()        { return ObjectNode(*this, UA_NS0ID_DATATYPESFOLDER); }
ObjectNode Server::getReferenceTypesNode()   { return ObjectNode(*this, UA_NS0ID_REFERENCETYPESFOLDER); }

Server::Connection::Connection()
    : server_(UA_Server_new()) {
    UA_ServerConfig_setDefault(getConfig());

    // change default parameters
    auto* config = getConfig();
    config->publishingIntervalLimits.min = 10; // ms
    config->samplingIntervalLimits.min = 10; // ms
}

Server::Connection::~Connection() {
    stop();
    UA_Server_delete(server_);
}

void Server::Connection::run() {
    if (running_.load())
        throw Exception("OPC UA Server already running");

    auto status = UA_Server_run_startup(server_);
    if (status != UA_STATUSCODE_GOOD)
        throw Exception(status);

    running_.store(true);

    while (this->running_.load()) {
        // references: 
        // https://open62541.org/doc/current/server.html#server-lifecycle
        // https://github.com/open62541/open62541/blob/master/examples/server_mainloop.c
        const auto waitInterval = UA_Server_run_iterate(this->server_, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(waitInterval));
    }
}

void Server::Connection::stop() {
    if (!running_.load())
        return;

    running_.store(false);

    auto status = UA_Server_run_shutdown(this->server_);
    if (status != UA_STATUSCODE_GOOD)
        throw Exception(status);
}

UA_ServerConfig* Server::Connection::getConfig() {
    return UA_Server_getConfig(server_);
}

} // namespace opcua
