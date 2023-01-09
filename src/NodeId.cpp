#include "open62541pp/NodeId.h"

#include <cassert>

#include "open62541pp/open62541.h"

namespace opcua {

static UA_NodeId fromStringView(
    UA_UInt16 namespaceIndex, UA_NodeIdType identifierType, std::string_view identifier
) {
    // NOLINTNEXTLINE
    assert(identifierType == UA_NODEIDTYPE_STRING || identifierType == UA_NODEIDTYPE_BYTESTRING);
    UA_NodeId result;
    result.namespaceIndex = namespaceIndex;
    result.identifierType = identifierType;
    result.identifier.string = detail::allocUaString(identifier);  // NOLINT
    return result;
}

NodeId::NodeId(UA_UInt32 identifier, UA_UInt16 namespaceIndex)
    : NodeId(UA_NODEID_NUMERIC(namespaceIndex, identifier)) {
}

NodeId::NodeId(std::string_view identifier, UA_UInt16 namespaceIndex)
    : NodeId(fromStringView(namespaceIndex, UA_NODEIDTYPE_STRING, identifier)) {
}

NodeId::NodeId(const String& identifier, UA_UInt16 namespaceIndex)
    : NodeId(identifier.getView(), namespaceIndex) {
}

NodeId::NodeId(const Guid& identifier, UA_UInt16 namespaceIndex)
    : NodeId(UA_NODEID_GUID(namespaceIndex, *identifier.handle())) {
}

NodeId::NodeId(const ByteString& identifier, UA_UInt16 namespaceIndex)
    : NodeId(fromStringView(namespaceIndex, UA_NODEIDTYPE_BYTESTRING, identifier.getView())) {
}

bool NodeId::operator==(const NodeId& other) const {
    return UA_NodeId_order(handle(), other.handle()) == UA_ORDER_EQ;
}

bool NodeId::operator!=(const NodeId& other) const {
    return UA_NodeId_order(handle(), other.handle()) != UA_ORDER_EQ;
}

bool NodeId::operator<(const NodeId& other) const {
    return UA_NodeId_order(handle(), other.handle()) == UA_ORDER_LESS;
}

bool NodeId::operator>(const NodeId& other) const {
    return UA_NodeId_order(handle(), other.handle()) == UA_ORDER_MORE;
}

UA_UInt32 NodeId::hash() const {
    return UA_NodeId_hash(handle());
}

UA_UInt16 NodeId::getNamespaceIndex() const {
    return handle()->namespaceIndex;
}

UA_NodeIdType NodeId::getIdentifierType() const {
    return handle()->identifierType;
}

std::variant<UA_UInt32, String, Guid, ByteString> NodeId::getIdentifier() const {
    switch (handle()->identifierType) {
    case UA_NODEIDTYPE_NUMERIC:
        return handle()->identifier.numeric;  // NOLINT
    case UA_NODEIDTYPE_STRING:
        return String(handle()->identifier.string);  // NOLINT
    case UA_NODEIDTYPE_GUID:
        return Guid(handle()->identifier.guid);  // NOLINT
    case UA_NODEIDTYPE_BYTESTRING:
        return ByteString(handle()->identifier.byteString);  // NOLINT
    default:
        return {};
    }
}

}  // namespace opcua
