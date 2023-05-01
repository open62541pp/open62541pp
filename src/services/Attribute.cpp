#include "open62541pp/services/Attribute.h"

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"

#include "../open62541_impl.h"

namespace opcua::services {

template <>
DataValue readAttribute<Server>(
    Server& server, const NodeId& id, AttributeId attributeId, UA_TimestampsToReturn timestamps
) {
    UA_ReadValueId item{};
    item.nodeId = *id.handle();
    item.attributeId = static_cast<uint32_t>(attributeId);

    DataValue result = UA_Server_read(server.handle(), &item, timestamps);
    if (result->hasStatus) {
        detail::throwOnBadStatus(result->status);
    }
    return result;
}

template <>
DataValue readAttribute<Client>(
    Client& client, const NodeId& id, AttributeId attributeId, UA_TimestampsToReturn timestamps
) {
    // https://github.com/open62541/open62541/blob/v1.3.5/src/client/ua_client_highlevel.c#L357
    UA_ReadValueId item{};
    item.nodeId = *id.handle();
    item.attributeId = static_cast<uint32_t>(attributeId);

    UA_ReadRequest request{};
    request.timestampsToReturn = timestamps;
    request.nodesToReadSize = 1;
    request.nodesToRead = &item;

    using ReadResponse = TypeWrapper<UA_ReadResponse, UA_TYPES_READRESPONSE>;
    ReadResponse response = UA_Client_Service_read(client.handle(), request);
    detail::throwOnBadStatus(response->responseHeader.serviceResult);

    if (response->resultsSize != 1) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
    if (!response->results->hasValue) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
    if (response->results->hasStatus) {
        detail::throwOnBadStatus(response->results->status);
    }

    DataValue result;
    result.swap(*response->results);
    return result;
}

template <>
void writeAttribute<Server>(
    Server& server, const NodeId& id, AttributeId attributeId, const DataValue& value
) {
    UA_WriteValue item{};
    item.nodeId = *id.handle();
    item.attributeId = static_cast<uint32_t>(attributeId);
    item.value = *value.handle();  // shallow copy
    item.value.hasValue = true;

    const auto status = UA_Server_write(server.handle(), &item);
    detail::throwOnBadStatus(status);
}

template <>
void writeAttribute<Client>(
    Client& client, const NodeId& id, AttributeId attributeId, const DataValue& value
) {
    // https://github.com/open62541/open62541/blob/v1.3.5/src/client/ua_client_highlevel.c#L285
    UA_WriteValue item{};
    item.nodeId = *id.handle();
    item.attributeId = static_cast<uint32_t>(attributeId);
    item.value = *value.handle();  // shallow copy
    item.value.hasValue = true;

    UA_WriteRequest request{};
    request.nodesToWrite = &item;
    request.nodesToWriteSize = 1;

    using WriteResponse = TypeWrapper<UA_WriteResponse, UA_TYPES_WRITERESPONSE>;
    WriteResponse response = UA_Client_Service_write(client.handle(), request);
    detail::throwOnBadStatus(response->responseHeader.serviceResult);

    if (response->resultsSize != 1) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
    detail::throwOnBadStatus(*response->results);
}

}  // namespace opcua::services
