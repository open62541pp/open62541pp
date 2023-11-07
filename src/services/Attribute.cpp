#include "open62541pp/services/Attribute.h"

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"  // asWrapper, asNative

#include "../open62541_impl.h"
#include "ClientService.h"

namespace opcua::services {

ReadResponse read(Client& client, const ReadRequest& request) {
    return sendRequest<ReadRequest, ReadResponse>(client, request, ForwardResponse<ReadResponse>{});
}

inline static auto createReadValueId(const NodeId& id, AttributeId attributeId) {
    UA_ReadValueId item{};
    item.nodeId = *id.handle();
    item.attributeId = static_cast<uint32_t>(attributeId);
    return item;
}

inline static auto createReadRequest(
    TimestampsToReturn timestamps, Span<const ReadValueId> nodesToRead
) noexcept {
    UA_ReadRequest request{};
    request.timestampsToReturn = static_cast<UA_TimestampsToReturn>(timestamps);
    request.nodesToReadSize = nodesToRead.size();
    request.nodesToRead = asNative(const_cast<ReadValueId*>(nodesToRead.data()));  // NOLINT
    return request;
}

ReadResponse read(
    Client& client, Span<const ReadValueId> nodesToRead, TimestampsToReturn timestamps
) {
    const auto request = createReadRequest(timestamps, nodesToRead);
    return read(client, asWrapper<ReadRequest>(request));
}

template <>
DataValue readAttribute<Server>(
    Server& server, const NodeId& id, AttributeId attributeId, TimestampsToReturn timestamps
) {
    const auto item = createReadValueId(id, attributeId);
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
    const auto item = createReadValueId(id, attributeId);
    const auto request = createReadRequest(timestamps, {asWrapper<ReadValueId>(&item), 1});
    auto response = read(client, asWrapper<ReadRequest>(request));
    auto results = response.getResults();
    if (results.size() != 1 || !results[0]->hasValue) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
    if (results[0]->hasStatus) {
        detail::throwOnBadStatus(results[0]->status);
    }
    DataValue result;
    result.swap(response.getResults()[0]);
    return result;
}

WriteResponse write(Client& client, const WriteRequest& request) {
    return sendRequest<WriteRequest, WriteResponse>(
        client, request, ForwardResponse<WriteResponse>{}
    );
}

WriteResponse write(Client& client, Span<const WriteValue> nodesToWrite) {
    // avoid copy of nodesToWrite
    UA_WriteRequest request{};
    request.nodesToWriteSize = nodesToWrite.size();
    request.nodesToWrite = asNative(const_cast<WriteValue*>(nodesToWrite.data()));  // NOLINT
    return write(client, asWrapper<WriteRequest>(request));
}

inline static UA_WriteValue createWriteValue(
    const NodeId& id, AttributeId attributeId, const DataValue& value
) noexcept {
    UA_WriteValue item{};
    item.nodeId = *id.handle();
    item.attributeId = static_cast<uint32_t>(attributeId);
    item.value = *value.handle();  // shallow copy, avoid copy of value
    item.value.hasValue = true;
    return item;
}

template <>
void writeAttribute<Server>(
    Server& server, const NodeId& id, AttributeId attributeId, const DataValue& value
) {
    const auto item = createWriteValue(id, attributeId, value);
    const auto status = UA_Server_write(server.handle(), &item);
    detail::throwOnBadStatus(status);
}

template <>
void writeAttribute<Client>(
    Client& client, const NodeId& id, AttributeId attributeId, const DataValue& value
) {
    const auto item = createWriteValue(id, attributeId, value);
    auto response = write(client, {asWrapper<WriteValue>(&item), 1});
    auto results = response.getResults();
    if (results.size() != 1) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
    detail::throwOnBadStatus(results[0]);
}

}  // namespace opcua::services
