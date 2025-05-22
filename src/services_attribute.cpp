#include "open62541pp/services/attribute.hpp"

#include "open62541pp/client.hpp"
#include "open62541pp/server.hpp"

namespace opcua::services {

ReadResponse read(Client& connection, const ReadRequest& request) noexcept {
    return UA_Client_Service_read(connection.handle(), request);
}

template <>
Result<DataValue> readAttribute<Server>(
    Server& connection, const NodeId& id, AttributeId attributeId, TimestampsToReturn timestamps
) noexcept {
    const auto item = detail::makeReadValueId(id, attributeId);
    return DataValue(
        UA_Server_read(connection.handle(), &item, static_cast<UA_TimestampsToReturn>(timestamps))
    );
}

template <>
Result<DataValue> readAttribute<Client>(
    Client& connection, const NodeId& id, AttributeId attributeId, TimestampsToReturn timestamps
) noexcept {
    auto item = detail::makeReadValueId(id, attributeId);
    const auto request = detail::makeReadRequest(timestamps, item);
    auto response = read(connection, asWrapper<ReadRequest>(request));
    return detail::wrapSingleResult<DataValue>(response);
}

WriteResponse write(Client& connection, const WriteRequest& request) noexcept {
    return UA_Client_Service_write(connection.handle(), request);
}

template <>
StatusCode writeAttribute<Server>(
    Server& connection, const NodeId& id, AttributeId attributeId, const DataValue& value
) noexcept {
    const auto item = detail::makeWriteValue(id, attributeId, value);
    return UA_Server_write(connection.handle(), &item);
}

template <>
StatusCode writeAttribute<Client>(
    Client& connection, const NodeId& id, AttributeId attributeId, const DataValue& value
) noexcept {
    auto item = detail::makeWriteValue(id, attributeId, value);
    const auto request = detail::makeWriteRequest(item);
    return detail::getSingleStatus(write(connection, asWrapper<WriteRequest>(request)));
}

}  // namespace opcua::services
