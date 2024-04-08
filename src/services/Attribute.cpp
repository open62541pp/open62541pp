#include "open62541pp/services/Attribute.h"

#include "open62541pp/Server.h"
#include "open62541pp/detail/open62541/server.h"

namespace opcua::services {

ReadResponse read(Client& connection, const ReadRequest& request) {
    return readAsync(connection, request, detail::SyncOperation{});
}

template <>
Result<DataValue> readAttribute<Server>(
    Server& connection, const NodeId& id, AttributeId attributeId, TimestampsToReturn timestamps
) {
    const auto item = detail::createReadValueId(id, attributeId);
    return DataValue(
        UA_Server_read(connection.handle(), &item, static_cast<UA_TimestampsToReturn>(timestamps))
    );
}

template <>
Result<DataValue> readAttribute<Client>(
    Client& connection, const NodeId& id, AttributeId attributeId, TimestampsToReturn timestamps
) {
    return readAttributeAsync(connection, id, attributeId, timestamps, detail::SyncOperation{});
}

WriteResponse write(Client& connection, const WriteRequest& request) {
    return writeAsync(connection, request, detail::SyncOperation{});
}

template <>
Result<void> writeAttribute<Server>(
    Server& connection, const NodeId& id, AttributeId attributeId, const DataValue& value
) {
    const auto item = detail::createWriteValue(id, attributeId, value);
    return detail::asResult(UA_Server_write(connection.handle(), &item));
}

template <>
Result<void> writeAttribute<Client>(
    Client& connection, const NodeId& id, AttributeId attributeId, const DataValue& value
) {
    return writeAttributeAsync(connection, id, attributeId, value, detail::SyncOperation{});
}

}  // namespace opcua::services
