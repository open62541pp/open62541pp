#include "open62541pp/Server.h"

// turn off the -Wunused-parameter warning for open62541 (only for gcc/clang)
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "open62541/types.h"
#include "open62541/types_generated_handling.h"
#include "open62541/server.h"
#if __has_include("open62541/server_config.h")  // merged into server.h in v1.2
#include "open62541/server_config.h"
#endif
#include "open62541/server_config_default.h"
#include "open62541/plugin/accesscontrol_default.h"

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

#include "open62541pp/Helper.h"
#include "open62541pp/Node.h"
#include "open62541pp/NodeId.h"
#include "open62541pp/ErrorHandling.h"

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

void Server::Connection::start() {
    if (running_.load())
        throw Exception("OPC UA Server already started");

    auto status = UA_Server_run_startup(server_);
    if (status != UA_STATUSCODE_GOOD)
        throw Exception(status);

    running_.store(true);
    thread_  = std::thread([this] {
        while (this->running_.load()) {
            // reference: https://open62541.org/doc/current/server.html#server-lifecycle
            UA_Server_run_iterate(this->server_, true);
        }
    });
}

void Server::Connection::stop() {
    if (!running_.load())
        return;

    running_.store(false);
    thread_.join();

    auto status = UA_Server_run_shutdown(server_);
    if (status != UA_STATUSCODE_GOOD)
        throw Exception(status);
}

UA_ServerConfig* Server::Connection::getConfig() {
    return UA_Server_getConfig(server_);
}

} // namespace opcua
