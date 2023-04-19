#include "open62541pp/types/NodeId.h"

#include <cassert>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/detail/helper.h"

#include "../open62541_impl.h"

namespace opcua {

static UA_NodeId fromStringView(
    uint16_t namespaceIndex, UA_NodeIdType identifierType, std::string_view identifier
) {
    // NOLINTNEXTLINE
    assert(identifierType == UA_NODEIDTYPE_STRING || identifierType == UA_NODEIDTYPE_BYTESTRING);
    UA_NodeId result;
    result.namespaceIndex = namespaceIndex;
    result.identifierType = identifierType;
    result.identifier.string = detail::allocUaString(identifier);  // NOLINT
    return result;
}

NodeId::NodeId(uint16_t namespaceIndex, uint32_t identifier)
    : NodeId(UA_NODEID_NUMERIC(namespaceIndex, identifier)) {}

NodeId::NodeId(uint16_t namespaceIndex, std::string_view identifier)
    : NodeId(fromStringView(namespaceIndex, UA_NODEIDTYPE_STRING, identifier)) {}

NodeId::NodeId(uint16_t namespaceIndex, const String& identifier)
    : NodeId(namespaceIndex, identifier.get()) {}

NodeId::NodeId(uint16_t namespaceIndex, const Guid& identifier)
    : NodeId(UA_NODEID_GUID(namespaceIndex, *identifier.handle())) {}

NodeId::NodeId(uint16_t namespaceIndex, const ByteString& identifier)
    : NodeId(fromStringView(namespaceIndex, UA_NODEIDTYPE_BYTESTRING, identifier.get())) {}

uint32_t NodeId::hash() const {
    return UA_NodeId_hash(handle());
}

uint16_t NodeId::getNamespaceIndex() const noexcept {
    return handle()->namespaceIndex;
}

NodeIdType NodeId::getIdentifierType() const noexcept {
    return static_cast<NodeIdType>(handle()->identifierType);
}

std::variant<uint32_t, String, Guid, ByteString> NodeId::getIdentifier() const {
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

/* --------------------------------------- ExpandedNodeId --------------------------------------- */

ExpandedNodeId::ExpandedNodeId(
    const NodeId& id, std::string_view namespaceUri, uint32_t serverIndex
) {
    const auto status = UA_NodeId_copy(id.handle(), &handle()->nodeId);
    detail::throwOnBadStatus(status);
    handle()->namespaceUri = detail::allocUaString(namespaceUri);
    handle()->serverIndex = serverIndex;
}

bool ExpandedNodeId::isLocal() const noexcept {
    return detail::isEmpty(handle()->namespaceUri) && handle()->serverIndex == 0;
}

NodeId& ExpandedNodeId::getNodeId() noexcept {
    return asWrapper<NodeId>(handle()->nodeId);
}

const NodeId& ExpandedNodeId::getNodeId() const noexcept {
    return asWrapper<NodeId>(handle()->nodeId);
}

std::string_view ExpandedNodeId::getNamespaceUri() const {
    return detail::toStringView(handle()->namespaceUri);
}

uint32_t ExpandedNodeId::getServerIndex() const noexcept {
    return handle()->serverIndex;
}

}  // namespace opcua
