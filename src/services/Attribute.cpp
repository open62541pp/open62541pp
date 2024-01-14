#include "open62541pp/services/Attribute.h"

#include "open62541pp/Server.h"

#include "../open62541_impl.h"

namespace opcua::services {

ReadResponse read(Client& client, const ReadRequest& request) {
    return readAsync(client, request, detail::SyncOperation{});
}

template <>
DataValue readAttribute<Server>(
    Server& server, const NodeId& id, AttributeId attributeId, TimestampsToReturn timestamps
) {
    const auto item = detail::createReadValueId(id, attributeId);
    auto result = UA_Server_read(
        server.handle(), &item, static_cast<UA_TimestampsToReturn>(timestamps)
    );
    detail::checkReadResult(result);
    return result;
}

template <>
DataValue readAttribute<Client>(
    Client& client, const NodeId& id, AttributeId attributeId, TimestampsToReturn timestamps
) {
    return readAttributeAsync(client, id, attributeId, timestamps, detail::SyncOperation{});
}

WriteResponse write(Client& client, const WriteRequest& request) {
    return writeAsync(client, request, detail::SyncOperation{});
}

template <>
void writeAttribute<Server>(
    Server& server, const NodeId& id, AttributeId attributeId, const DataValue& value
) {
    const auto item = detail::createWriteValue(id, attributeId, value);
    const auto status = UA_Server_write(server.handle(), &item);
    opcua::detail::throwOnBadStatus(status);
}

template <>
void writeAttribute<Client>(
    Client& client, const NodeId& id, AttributeId attributeId, const DataValue& value
) {
    writeAttributeAsync(client, id, attributeId, value, detail::SyncOperation{});
}

}  // namespace opcua::services
