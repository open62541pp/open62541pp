#include "open62541pp/types/NodeId.h"

#include <cassert>

#include "open62541pp/detail/helper.h"

namespace opcua {

NodeId::NodeId(NamespaceIndex namespaceIndex, uint32_t identifier) noexcept {
    handle()->namespaceIndex = namespaceIndex;
    handle()->identifierType = UA_NODEIDTYPE_NUMERIC;
    handle()->identifier.numeric = identifier;  // NOLINT
}

NodeId::NodeId(NamespaceIndex namespaceIndex, std::string_view identifier) {
    handle()->namespaceIndex = namespaceIndex;
    handle()->identifierType = UA_NODEIDTYPE_STRING;
    handle()->identifier.string = detail::allocNativeString(identifier);  // NOLINT
}

NodeId::NodeId(NamespaceIndex namespaceIndex, String identifier) noexcept {
    handle()->namespaceIndex = namespaceIndex;
    handle()->identifierType = UA_NODEIDTYPE_STRING;
    handle()->identifier.string = std::exchange(asNative(identifier), {});  // NOLINT
}

NodeId::NodeId(NamespaceIndex namespaceIndex, Guid identifier) noexcept {
    handle()->namespaceIndex = namespaceIndex;
    handle()->identifierType = UA_NODEIDTYPE_GUID;
    handle()->identifier.guid = identifier;  // NOLINT
}

NodeId::NodeId(NamespaceIndex namespaceIndex, ByteString identifier) noexcept {
    handle()->namespaceIndex = namespaceIndex;
    handle()->identifierType = UA_NODEIDTYPE_BYTESTRING;
    handle()->identifier.byteString = std::exchange(asNative(identifier), {});  // NOLINT
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

ExpandedNodeId::ExpandedNodeId(NodeId id) noexcept {
    asWrapper<NodeId>(handle()->nodeId) = std::move(id);
}

ExpandedNodeId::ExpandedNodeId(NodeId id, std::string_view namespaceUri, uint32_t serverIndex) {
    asWrapper<NodeId>(handle()->nodeId) = std::move(id);
    handle()->namespaceUri = detail::allocNativeString(namespaceUri);
    handle()->serverIndex = serverIndex;
}

bool ExpandedNodeId::isLocal() const noexcept {
    return detail::isEmpty(handle()->namespaceUri) && handle()->serverIndex == 0;
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
