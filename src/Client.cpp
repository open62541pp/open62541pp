#include "open62541pp/Client.h"
#include "open62541pp/Helper.h"

#include "open62541pp/Logger.h"

#include <open62541/client_config_default.h>

namespace opcua {

// https://stackoverflow.com/questions/58316274/open62541-browsing-nodes-an-using-its-methods

struct Client::PrivateData {
    UA_Client* client{nullptr};
};

Client::Client()
    : d_(new Client::PrivateData) {
    d_->client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(d_->client));
}

Client::~Client() {
    if (d_->client)
        UA_Client_delete(d_->client);
}

std::vector<std::pair<std::string, std::string>> Client::findServers(std::string_view url) {
    size_t nServers{0};
    UA_ApplicationDescription* registeredServers = nullptr;

    if (detail::isBadStatus(UA_Client_findServers(
            d_->client, url.data(), 0, nullptr, 0, nullptr, &nServers, &registeredServers
        )))
        return {};

    std::vector<std::pair<std::string, std::string>> res;

    for (size_t i = 0; i < nServers; ++i) {
        res.emplace_back(std::pair<std::string, std::string>{
            detail::toString(registeredServers[i].applicationName.text),
            detail::toString(*registeredServers[i].discoveryUrls)});
    }

    UA_Array_delete(registeredServers, nServers, &UA_TYPES[UA_TYPES_APPLICATIONDESCRIPTION]);
    return res;
}

std::vector<Endpoint> Client::getEndpoints(std::string_view url) {
    UA_EndpointDescription* descArray = nullptr;
    size_t descArraySize = 0;

    detail::throwOnBadStatus(
        UA_Client_getEndpoints(d_->client, url.data(), &descArraySize, &descArray)
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
    detail::throwOnBadStatus(UA_Client_connect(d_->client, url.data()));
}

void Client::connect(std::string_view url, std::string_view username, std::string_view password) {
    detail::throwOnBadStatus(
        UA_Client_connectUsername(d_->client, url.data(), username.data(), password.data())
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
    return d_->client;
}

const UA_Client* Client::handle() const noexcept {
    return d_->client;
}
} // namespace opcua