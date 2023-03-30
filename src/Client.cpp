#include "open62541pp/Client.h"
#include "open62541pp/Helper.h"

#include "open62541pp/Logger.h"

#include <open62541/client_config_default.h>

using namespace opcua;

struct Client::PrivateData {
    UA_Client* client{nullptr};
};

Client::Client()
    : m_d(new Client::PrivateData) {
    m_d->client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(m_d->client));
}

Client::~Client() {
    if (m_d->client)
        UA_Client_delete(m_d->client);
}

std::vector<std::pair<std::string, std::string>> Client::findServers(std::string_view url) {
    size_t nServers{0};
    UA_ApplicationDescription* registeredServers = nullptr;

    if (detail::isBadStatus(UA_Client_findServers(
            m_d->client, url.data(), 0, nullptr, 0, nullptr, &nServers, &registeredServers
        )))
        return {};

    std::vector<std::pair<std::string, std::string>> res;

    for (size_t i = 0; i < nServers; ++i) {
        res.emplace_back(std::pair<std::string, std::string>{
            detail::toString(registeredServers[i].applicationName.text),
            detail::toString(registeredServers[i].applicationUri)});
    }

    UA_Array_delete(registeredServers, nServers, &UA_TYPES[UA_TYPES_APPLICATIONDESCRIPTION]);
    return res;
}

std::vector<Endpoint> Client::getEndpoints(std::string_view url) {
    UA_EndpointDescription* descArray = nullptr;
    size_t descArraySize = 0;

    detail::throwOnBadStatus(
        UA_Client_getEndpoints(m_d->client, url.data(), &descArraySize, &descArray)
    );

    std::vector<Endpoint> ret;

    for (size_t i = 0; i < descArraySize; ++i) {
        Endpoint point;
        point.url = detail::toString(descArray[i].endpointUrl);
        TypeConverter<ApplicationDescription>::fromNative(descArray[i].server, point.server);
        point.serverCertificate = detail::toString(descArray[i].serverCertificate);

        ret.emplace_back(std::move(point));
    }

    UA_Array_delete(descArray, descArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
    return ret;
}

void Client::connect(std::string_view url) {
    detail::throwOnBadStatus(UA_Client_connect(m_d->client, url.data()));
}

void Client::connect(std::string_view url, std::string_view username, std::string_view password) {
    detail::throwOnBadStatus(
        UA_Client_connectUsername(m_d->client, url.data(), username.data(), password.data())
    );
}

Variant Client::readValueAttribute(const NodeId& nodeId) {
    (void)nodeId;
    return {};
}

void Client::writeValueAttribute(const NodeId& nodeId, const Variant& value) {
    (void)nodeId;
    (void)value;
}

UA_Client* Client::handle() noexcept {
    return m_d->client;
}

const UA_Client* Client::handle() const noexcept {
    return m_d->client;
}

bool Client::connected() {
    UA_SecureChannelState* chanState = nullptr;
    UA_SessionState* sessionState = nullptr;
    UA_StatusCode* connectStatus = nullptr;

    UA_Client_getState(m_d->client, chanState, sessionState, connectStatus);
    detail::throwOnBadStatus(*connectStatus);

    return *connectStatus == UA_SESSIONSTATE_ACTIVATED;
}