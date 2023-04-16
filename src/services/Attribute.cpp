#include "open62541pp/services/Attribute.h"

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"

#include "../open62541_impl.h"

namespace opcua::services {

DataValue readAttribute(
    Server& server, const NodeId& id, UA_AttributeId attributeId, UA_TimestampsToReturn timestamps
) {
    UA_ReadValueId item{};
    item.nodeId = *id.handle();
    item.attributeId = attributeId;
    DataValue dv(UA_Server_read(server.handle(), &item, timestamps));
    detail::throwOnBadStatus(dv.getStatusCode().value_or(UA_STATUSCODE_GOOD));
    return dv;
}

void writeAttribute(
    Server& server, const NodeId& id, UA_AttributeId attributeId, const DataValue& value
) {
    UA_WriteValue item{};
    item.nodeId = *id.handle();
    item.attributeId = attributeId;
    item.value = *value.handle();  // shallow copy
    const auto status = UA_Server_write(server.handle(), &item);
    detail::throwOnBadStatus(status);
}

}  // namespace opcua::services
