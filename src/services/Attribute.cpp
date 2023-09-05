#include "open62541pp/services/Attribute.h"

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"  // asWrapper, asNative

#include "../open62541_impl.h"

namespace opcua::services {

ReadResponse read(Client& client, const ReadRequest& request) {
    ReadResponse response = UA_Client_Service_read(client.handle(), request);
    detail::throwOnBadStatus(response->responseHeader.serviceResult);
    return response;
}

ReadResponse read(
    Client& client, Span<const ReadValueId> nodesToRead, TimestampsToReturn timestamps
) {
    // avoid copy of nodesToRead
    UA_ReadRequest request{};
    request.timestampsToReturn = static_cast<UA_TimestampsToReturn>(timestamps);
    request.nodesToReadSize = nodesToRead.size();
    request.nodesToRead = asNative(const_cast<ReadValueId*>(nodesToRead.data()));  // NOLINT
    return read(client, asWrapper<ReadRequest>(request));
}

template <>
DataValue readAttribute<Server>(
    Server& server, const NodeId& id, AttributeId attributeId, TimestampsToReturn timestamps
) {
    UA_ReadValueId item{};
    item.nodeId = *id.handle();
    item.attributeId = static_cast<uint32_t>(attributeId);

    DataValue result = UA_Server_read(
        server.handle(), &item, static_cast<UA_TimestampsToReturn>(timestamps)
    );
    if (result->hasStatus) {
        detail::throwOnBadStatus(result->status);
    }
    return result;
}

template <>
DataValue readAttribute<Client>(
    Client& client, const NodeId& id, AttributeId attributeId, TimestampsToReturn timestamps
) {
    auto response = read(client, {{id, attributeId}}, timestamps);
    auto results = response.getResults();
    if (results.size() != 1 || !results[0]->hasValue) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
    if (results[0]->hasStatus) {
        detail::throwOnBadStatus(results[0]->status);
    }
    DataValue result;
    result.swap(*response->results);  // TODO: non-const response.getResults()
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
