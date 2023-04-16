#include "open62541pp/Client.h"

#include <string>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Node.h"
#include "open62541pp/TypeConverter.h"

#include "open62541_impl.h"
#include "version.h"

namespace opcua {

/* ----------------------------------------- Connection ----------------------------------------- */

class Client::Connection {
public:
    Connection()
        : client_(UA_Client_new()) {}

    ~Connection() {
        UA_Client_disconnect(client_);
        UA_Client_delete(client_);
    }

    // prevent copy & move
    Connection(const Connection&) = delete;
    Connection(Connection&&) noexcept = delete;
    Connection& operator=(const Connection&) = delete;
    Connection& operator=(Connection&&) noexcept = delete;

    UA_Client* handle() noexcept {
        return client_;
    }

private:
    UA_Client* client_;
};

/* ------------------------------------------- Client ------------------------------------------- */

Client::Client()
    : connection_(std::make_shared<Connection>()) {
    const auto status = UA_ClientConfig_setDefault(UA_Client_getConfig(handle()));
    detail::throwOnBadStatus(status);
}

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

Node<Client> Client::getNode(const NodeId& id) {
    return {*this, id, true};
}

Node<Client> Client::getRootNode() {
    return {*this, {0, UA_NS0ID_ROOTFOLDER}, false};
}

Node<Client> Client::getObjectsNode() {
    return {*this, {0, UA_NS0ID_OBJECTSFOLDER}, false};
}

Node<Client> Client::getTypesNode() {
    return {*this, {0, UA_NS0ID_TYPESFOLDER}, false};
}

Node<Client> Client::getViewsNode() {
    return {*this, {0, UA_NS0ID_VIEWSFOLDER}, false};
}

Node<Client> Client::getObjectTypesNode() {
    return {*this, {0, UA_NS0ID_OBJECTTYPESFOLDER}, false};
}

Node<Client> Client::getVariableTypesNode() {
    return {*this, {0, UA_NS0ID_VARIABLETYPESFOLDER}, false};
}

Node<Client> Client::getDataTypesNode() {
    return {*this, {0, UA_NS0ID_DATATYPESFOLDER}, false};
}

Node<Client> Client::getReferenceTypesNode() {
    return {*this, {0, UA_NS0ID_REFERENCETYPESFOLDER}, false};
}

Node<Client> Client::getBaseObjectTypeNode() {
    return {*this, {0, UA_NS0ID_BASEOBJECTTYPE}, false};
}

Node<Client> Client::getBaseDataVariableTypeNode() {
    return {*this, {0, UA_NS0ID_BASEDATAVARIABLETYPE}, false};
}

UA_Client* Client::handle() noexcept {
    return connection_->handle();
}

const UA_Client* Client::handle() const noexcept {
    return connection_->handle();
}

}  // namespace opcua
