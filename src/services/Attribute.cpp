#include "open62541pp/services/Attribute.h"

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Helper.h"
#include "open62541pp/Server.h"

#include "../open62541_impl.h"
#include "../version.h"

namespace opcua::services {

NodeClass readNodeClass(Server& server, const NodeId& id) {
    UA_NodeClass nodeClass = UA_NODECLASS_UNSPECIFIED;
    const auto status = UA_Server_readNodeClass(server.handle(), id, &nodeClass);
    detail::throwOnBadStatus(status);
    return static_cast<NodeClass>(nodeClass);
}

std::string readBrowseName(Server& server, const NodeId& id) {
    QualifiedName name;
    const auto status = UA_Server_readBrowseName(server.handle(), id, name.handle());
    detail::throwOnBadStatus(status);
    return std::string{name.getName()};
}

LocalizedText readDisplayName(Server& server, const NodeId& id) {
    LocalizedText text;
    const auto status = UA_Server_readDisplayName(server.handle(), id, text.handle());
    detail::throwOnBadStatus(status);
    return text;
}

LocalizedText readDescription(Server& server, const NodeId& id) {
    LocalizedText text;
    const auto status = UA_Server_readDescription(server.handle(), id, text.handle());
    detail::throwOnBadStatus(status);
    return text;
}

uint32_t readWriteMask(Server& server, const NodeId& id) {
    uint32_t writeMask = 0;
    const auto status = UA_Server_readWriteMask(server.handle(), id, &writeMask);
    detail::throwOnBadStatus(status);
    return writeMask;
}

NodeId readDataType(Server& server, const NodeId& id) {
    NodeId nodeId(0, 0);
    const auto status = UA_Server_readDataType(server.handle(), id, nodeId.handle());
    detail::throwOnBadStatus(status);
    return nodeId;
}

ValueRank readValueRank(Server& server, const NodeId& id) {
    int32_t valueRank = 0;
    const auto status = UA_Server_readValueRank(server.handle(), id, &valueRank);
    detail::throwOnBadStatus(status);
    return static_cast<ValueRank>(valueRank);
}

std::vector<uint32_t> readArrayDimensions(Server& server, const NodeId& id) {
    Variant variant;
    const auto status = UA_Server_readArrayDimensions(server.handle(), id, variant.handle());
    detail::throwOnBadStatus(status);
    if (variant.isArray()) {
        return variant.getArrayCopy<uint32_t>();
    }
    return {};
}

uint8_t readAccessLevel(Server& server, const NodeId& id) {
    uint8_t mask = 0;
    const auto status = UA_Server_readAccessLevel(server.handle(), id, &mask);
    detail::throwOnBadStatus(status);
    return mask;
}

void readDataValue(Server& server, const NodeId& id, DataValue& value) {
    UA_ReadValueId rvi;
    UA_ReadValueId_init(&rvi);
    rvi.nodeId = *id.handle();
    rvi.attributeId = UA_ATTRIBUTEID_VALUE;
    value = DataValue(UA_Server_read(server.handle(), &rvi, UA_TIMESTAMPSTORETURN_BOTH));
    if (value->hasStatus) {
        detail::throwOnBadStatus(value->status);
    }
}

void readValue(Server& server, const NodeId& id, Variant& value) {
    const auto status = UA_Server_readValue(server.handle(), id, value.handle());
    detail::throwOnBadStatus(status);
}

void writeDisplayName(
    Server& server, const NodeId& id, std::string_view locale, std::string_view name
) {
    const auto status = UA_Server_writeDisplayName(
        server.handle(), id, LocalizedText(locale, name)
    );
    detail::throwOnBadStatus(status);
}

void writeDescription(
    Server& server, const NodeId& id, std::string_view locale, std::string_view name
) {
    const auto status = UA_Server_writeDescription(
        server.handle(), id, LocalizedText(locale, name)
    );
    detail::throwOnBadStatus(status);
}

void writeWriteMask(Server& server, const NodeId& id, uint32_t mask) {
    const auto status = UA_Server_writeWriteMask(server.handle(), id, mask);
    detail::throwOnBadStatus(status);
}

void writeDataType(Server& server, const NodeId& id, Type type) {
    const auto status = UA_Server_writeDataType(
        server.handle(), id, detail::getUaDataType(type)->typeId
    );
    detail::throwOnBadStatus(status);
}

void writeDataType(Server& server, const NodeId& id, const NodeId& typeId) {
    const auto status = UA_Server_writeDataType(server.handle(), id, typeId);
    detail::throwOnBadStatus(status);
}

void writeValueRank(Server& server, const NodeId& id, ValueRank valueRank) {
    const auto status = UA_Server_writeValueRank(
        server.handle(), id, static_cast<int32_t>(valueRank)
    );
    detail::throwOnBadStatus(status);
}

void writeArrayDimensions(
    Server& server, const NodeId& id, const std::vector<uint32_t>& dimensions
) {
    Variant variant;
    variant.setArrayCopy(dimensions);
    const auto status = UA_Server_writeArrayDimensions(server.handle(), id, variant);
    detail::throwOnBadStatus(status);
}

void writeAccessLevel(Server& server, const NodeId& id, uint8_t mask) {
    const auto status = UA_Server_writeAccessLevel(server.handle(), id, static_cast<UA_Byte>(mask));
    detail::throwOnBadStatus(status);
}

void writeModellingRule(Server& server, const NodeId& id, ModellingRule rule) {
    const auto status = UA_Server_addReference(
        server.handle(),  // server
        id,  // source id
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),  // reference id
        UA_EXPANDEDNODEID_NUMERIC(0, static_cast<UA_UInt32>(rule)),  // target id
        true  // forward
    );
    detail::throwOnBadStatus(status);
}

void writeDataValue(Server& server, const NodeId& id, const DataValue& value) {
    // support for types with optional fields introduced in v1.1
#if UAPP_OPEN62541_VER_GE(1, 1)
    const auto status = UA_Server_writeDataValue(server.handle(), id, value);
    detail::throwOnBadStatus(status);
#endif
}

void writeValue(Server& server, const NodeId& id, const Variant& value) {
    const auto status = UA_Server_writeValue(server.handle(), id, value);
    detail::throwOnBadStatus(status);
}

}  // namespace opcua::services
