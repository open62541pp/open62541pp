#include "open62541pp/services/attribute.hpp"

#include "open62541pp/client.hpp"
#include "open62541pp/detail/open62541/server.h"
#include "open62541pp/server.hpp"

namespace opcua::services {

ReadResponse read(Client& connection, const ReadRequest& request) noexcept {
    return readAsync(connection, request, detail::SyncOperation{});
}

template <>
Result<DataValue> readAttribute<Server>(
    Server& connection, const NodeId& id, AttributeId attributeId, TimestampsToReturn timestamps
) noexcept {
    const auto item = detail::createReadValueId(id, attributeId);
    return DataValue(
        UA_Server_read(connection.handle(), &item, static_cast<UA_TimestampsToReturn>(timestamps))
    );
}

template <>
Result<DataValue> readAttribute<Client>(
    Client& connection, const NodeId& id, AttributeId attributeId, TimestampsToReturn timestamps
) noexcept {
    return readAttributeAsync(connection, id, attributeId, timestamps, detail::SyncOperation{});
}

WriteResponse write(Client& connection, const WriteRequest& request) noexcept {
    return writeAsync(connection, request, detail::SyncOperation{});
}

template <>
Result<void> writeAttribute<Server>(
    Server& connection, const NodeId& id, AttributeId attributeId, const DataValue& value
) noexcept {
    const auto item = detail::createWriteValue(id, attributeId, value);
    return detail::toResult(UA_Server_write(connection.handle(), &item));
}

template <>
Result<void> writeAttribute<Client>(
    Client& connection, const NodeId& id, AttributeId attributeId, const DataValue& value
) noexcept {
    return writeAttributeAsync(connection, id, attributeId, value, detail::SyncOperation{});
}

}  // namespace opcua::services
