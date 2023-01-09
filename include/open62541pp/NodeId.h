#pragma once

#include <cassert>
#include <string_view>
#include <variant>

#include "open62541pp/open62541.h"

#include "TypeWrapper.h"

namespace opcua {

class NodeId : public TypeWrapper<UA_NodeId> {
public:
    using BaseClass::BaseClass;  // inherit contructors

    /// Create NodeId with numeric identifier
    NodeId(UA_UInt32 identifier, UA_UInt16 namespaceIndex)
        : NodeId(UA_NODEID_NUMERIC(namespaceIndex, identifier)) {}

    /// Create NodeId with String identifier from standard strings
    NodeId(std::string_view identifier, UA_UInt16 namespaceIndex)
        : NodeId(fromStringView(namespaceIndex, UA_NODEIDTYPE_STRING, identifier)) {}

    /// Create NodeId with String identifier from String wrapper class
    NodeId(const String& identifier, UA_UInt16 namespaceIndex)
        : NodeId(identifier.getView(), namespaceIndex) {}

    /// Create NodeId with Guid identifier
    NodeId(const Guid& identifier, UA_UInt16 namespaceIndex)
        : NodeId(UA_NODEID_GUID(namespaceIndex, *identifier.handle())) {}

    /// Create NodeId with ByteString identifier
    NodeId(const ByteString& identifier, UA_UInt16 namespaceIndex)
        : NodeId(fromStringView(namespaceIndex, UA_NODEIDTYPE_BYTESTRING, identifier.getView())) {}

    bool operator==(const NodeId& other) const {
        return UA_NodeId_order(handle(), other.handle()) == UA_ORDER_EQ;
    }

    bool operator!=(const NodeId& other) const {
        return UA_NodeId_order(handle(), other.handle()) != UA_ORDER_EQ;
    }

    bool operator<(const NodeId& other) const {
        return UA_NodeId_order(handle(), other.handle()) == UA_ORDER_LESS;
    }

    bool operator>(const NodeId& other) const {
        return UA_NodeId_order(handle(), other.handle()) == UA_ORDER_MORE;
    }

    UA_UInt32 hash() const { return UA_NodeId_hash(handle()); }

    UA_UInt16 getNamespaceIndex() const { return handle()->namespaceIndex; }

    UA_NodeIdType getIdentifierType() const { return handle()->identifierType; }

    std::variant<UA_UInt32, String, Guid, ByteString> getIdentifier() const {
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

private:
    static UA_NodeId fromStringView(
        UA_UInt16 namespaceIndex, UA_NodeIdType identifierType, std::string_view identifier
    ) {
        // NOLINTNEXTLINE
        assert(
            identifierType == UA_NODEIDTYPE_STRING || identifierType == UA_NODEIDTYPE_BYTESTRING
        );
        UA_NodeId result;
        result.namespaceIndex = namespaceIndex;
        result.identifierType = identifierType;
        result.identifier.string = allocUaString(identifier);  // NOLINT
        return result;
    }
};

}  // namespace opcua
