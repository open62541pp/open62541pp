#include "open62541pp/types/NodeId.h"

#include <cassert>

#include "open62541pp/detail/helper.h"

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

NodeId::NodeId(uint16_t namespaceIndex, uint32_t identifier) noexcept
    : NodeId(UA_NODEID_NUMERIC(namespaceIndex, identifier)) {}

NodeId::NodeId(uint16_t namespaceIndex, std::string_view identifier)
    : NodeId(fromStringView(namespaceIndex, UA_NODEIDTYPE_STRING, identifier)) {}

NodeId::NodeId(uint16_t namespaceIndex, const String& identifier)
    : NodeId(namespaceIndex, identifier.get()) {}

NodeId::NodeId(uint16_t namespaceIndex, const Guid& identifier)
    : NodeId(UA_NODEID_GUID(namespaceIndex, *identifier.handle())) {}

NodeId::NodeId(uint16_t namespaceIndex, const ByteString& identifier)
    : NodeId(fromStringView(namespaceIndex, UA_NODEIDTYPE_BYTESTRING, identifier.get())) {}

bool NodeId::isNull() const noexcept {
    return UA_NodeId_isNull(handle());
}

uint32_t NodeId::hash() const noexcept {
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

std::string NodeId::toString() const {
    std::string result;
    const auto ns = getNamespaceIndex();
    if (ns > 0) {
        result.append("ns=").append(std::to_string(ns)).append(";");
    }
    switch (getIdentifierType()) {
    case NodeIdType::Numeric:
        result.append("i=").append(std::to_string(getIdentifierAs<uint32_t>()));
        break;
    case NodeIdType::String:
        result.append("s=").append(getIdentifierAs<String>().get());
        break;
    case NodeIdType::Guid:
        result.append("g=").append(getIdentifierAs<Guid>().toString());
        break;
    case NodeIdType::ByteString:
        result.append("b=").append(getIdentifierAs<ByteString>().toBase64());
        break;
    }
    return result;
}

/* --------------------------------------- ExpandedNodeId --------------------------------------- */

ExpandedNodeId::ExpandedNodeId(const NodeId& id) {
    getNodeId() = id;
}

ExpandedNodeId::ExpandedNodeId(
    const NodeId& id, std::string_view namespaceUri, uint32_t serverIndex
) {
    getNodeId() = id;
    handle()->namespaceUri = detail::allocUaString(namespaceUri);
    handle()->serverIndex = serverIndex;
}

bool ExpandedNodeId::isLocal() const noexcept {
    return detail::isEmpty(handle()->namespaceUri) && handle()->serverIndex == 0;
}

uint32_t ExpandedNodeId::hash() const noexcept {
    return UA_ExpandedNodeId_hash(handle());
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

std::string ExpandedNodeId::toString() const {
    std::string result;
    const auto svr = getServerIndex();
    if (svr > 0) {
        result.append("svr=").append(std::to_string(svr)).append(";");
    }
    const auto nsu = getNamespaceUri();
    if (!nsu.empty()) {
        result.append("nsu=").append(nsu).append(";");
    }
    result.append(getNodeId().toString());
    return result;
}

}  // namespace opcua
