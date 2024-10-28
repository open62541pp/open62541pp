#include "open62541pp/node.hpp"

#include "open62541pp/client.hpp"
#include "open62541pp/detail/open62541/client.h"
#include "open62541pp/detail/open62541/server.h"
#include "open62541pp/server.hpp"

namespace opcua {

template <>
bool Node<Client>::exists() noexcept {
    // create minimal request
    UA_ReadValueId item{};
    item.nodeId = id();
    item.attributeId = UA_ATTRIBUTEID_NODECLASS;
    UA_ReadRequest request{};
    request.timestampsToReturn = UA_TIMESTAMPSTORETURN_NEITHER;
    request.nodesToReadSize = 1;
    request.nodesToRead = &item;

    const ReadResponse response = UA_Client_Service_read(connection().handle(), request);
    if (response->responseHeader.serviceResult != UA_STATUSCODE_GOOD ||
        response->resultsSize != 1) {
        return false;
    }
    if (response->results->hasStatus && response->results->status != UA_STATUSCODE_GOOD) {
        return false;
    }
    return true;
}

template <>
bool Node<Server>::exists() noexcept {
    void* context{};
    const auto status = UA_Server_getNodeContext(connection().handle(), id(), &context);
    return (status == UA_STATUSCODE_GOOD);
}

}  // namespace opcua
