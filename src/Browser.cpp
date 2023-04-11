// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "open62541pp/Browser.h"
#include "open62541pp/Client.h"
#include "open62541pp/types/ReferenceDescription.h"

#include "open62541/types.h"
#include "open62541/types_generated.h"
#include "open62541/nodeids.h"


namespace opcua {

struct Browser::PrivateData
{
    UA_BrowseRequest browseRequest;
    std::shared_ptr<Client> client;

};

Browser::~Browser() = default;

Browser::Browser (std::shared_ptr<Client> client_) : d_(new Browser::PrivateData)
{
    UA_BrowseRequest_init(&d_->browseRequest);
    d_->browseRequest.requestedMaxReferencesPerNode = 0;
    d_->browseRequest.nodesToBrowse = UA_BrowseDescription_new();
    d_->browseRequest.nodesToBrowseSize = 1;
    d_->browseRequest.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); /* browse objects folder */
    d_->browseRequest.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; /* return everything */

    d_->client = client_;
}

std::vector<NodeClient> Browser::browse() {
    std::vector<NodeClient> resp;

    UA_BrowseResponse bResp = UA_Client_Service_browse(d_->client->handle(), d_->browseRequest);
    for(size_t i = 0; i < bResp.resultsSize; ++i) {
        for (size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
            ReferenceDescription refDesc((bResp.results[i].references[j]));

            NodeClient node(d_->client, refDesc);
            resp.emplace_back(std::move(node));
        }
    }
    return resp;
}
} // namespace opcua