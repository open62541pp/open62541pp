#include "open62541pp/Client.h"
#include "open62541pp/Helper.h"

#include "open62541pp/Logger.h"

#include <open62541/client_config_default.h>

using namespace opcua;

struct Client::PrivateData {
	UA_Client* client{nullptr};
	UA_EndpointDescription *descArray nullptr;
	size_t descArraySize = 0;
};


Client::Client() : _d(new Client::PrivateData)
{
	_d->client = UA_Client_new();
	UA_ClientConfig_setDefault(UA_Client_getConfig(_d->client));
}

Client::~Client()
{

	UA_Client_delete(_d->client);
}

std::vector<std::pair<std::string, std::string>> Client::findServers(std::string_view url)
{
	size_t nServers{0};
	UA_ApplicationDescription *registeredServers = nullptr;

	if(detail::isBadStatus(UA_Client_findServers(_d->client, url.data(), 0, nullptr, 0, nullptr,
												 &nServers, &registeredServers)))
	   return {};

	vector<std::pair<std::string, std::string>> res(nServers);

	for (size_t i=0; i< nServers; ++i)
	{
		res.emplace_back(std::pair<std::string, std::string>{
				detail::toString(registeredServers[i].applicationName.text),
				detail::toString(registeredServers[i].applicationUri)});
	}

	UA_Array_delete(registeredServers, nServers, &UA_TYPES[UA_TYPES_APPLICATIONDESCRIPTION]);
}

Endpoints Client::getEndpoints(std::string_view url)
{
	detail::throwOnBadStatus(UA_Client_getEndpoints(_d->client, url, &descArraySize, &descArray));

	Endpoints ret;

	for (size_t i=0; i < descArraySize; ++i){
		ret.emplace_back(Endpoint{descArray[i].endpointUrl.data, descArray[i].endpointUrl.length});
	}

	UA_Array_delete(descArray, descArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
	return ret;
}
