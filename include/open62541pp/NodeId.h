#pragma once

#include <string_view>
#include <variant>

#include "open62541pp/open62541.h"

#include "TypeWrapper.h"

namespace opcua {

class NodeId : public TypeWrapper<UA_NodeId> {
public:
    using BaseClass::BaseClass;  // inherit contructors

    /// Create NodeId with numeric identifier
    NodeId(UA_UInt32 identifier, UA_UInt16 namespaceIndex = 0) {
        data_ = UA_NODEID_NUMERIC(namespaceIndex, identifier);
    }

    /// Create NodeId with String identifier from standard strings
    NodeId(std::string_view identifier, UA_UInt16 namespaceIndex = 0) {
        data_ = UA_NODEID_STRING_ALLOC(namespaceIndex, identifier.data());
    }

    /// Create NodeId with String identifier from String wrapper class
    NodeId(String identifier, UA_UInt16 namespaceIndex = 0) {
        data_ = UA_NODEID_STRING_ALLOC(namespaceIndex, identifier.get().data());
    }

    /// Create NodeId with Guid identifier
    NodeId(Guid identifier, UA_UInt16 namespaceIndex = 0) {
        data_ = UA_NODEID_GUID(namespaceIndex, *identifier.handle());
    }

    /// Create NodeId with ByteString identifier
    NodeId(ByteString identifier, UA_UInt16 namespaceIndex = 0) {
        data_ = UA_NODEID_BYTESTRING_ALLOC(namespaceIndex, identifier.get().data());
    }

    inline bool operator==(const NodeId& other) const {
        return UA_NodeId_order(&data_, other.handle()) == UA_ORDER_EQ;
    }

    inline bool operator!=(const NodeId& other) const {
        return UA_NodeId_order(&data_, other.handle()) != UA_ORDER_EQ;
    }

    inline bool operator<(const NodeId& other) const {
        return UA_NodeId_order(&data_, other.handle()) == UA_ORDER_LESS;
    }

    inline bool operator>(const NodeId& other) const {
        return UA_NodeId_order(&data_, other.handle()) == UA_ORDER_MORE;
    }

    inline UA_UInt32 hash() const { return UA_NodeId_hash(&data_); }

    inline UA_UInt16 getNamespaceIndex() const { return data_.namespaceIndex; }

    inline UA_NodeIdType getIdentifierType() const { return data_.identifierType; }

    std::variant<UA_UInt32, String, Guid, ByteString> getIdentifier() const {
        switch (data_.identifierType) {
        case UA_NODEIDTYPE_NUMERIC:
            return data_.identifier.numeric;
        case UA_NODEIDTYPE_STRING:
            return String(data_.identifier.string);
        case UA_NODEIDTYPE_GUID:
            return Guid(data_.identifier.guid);
        case UA_NODEIDTYPE_BYTESTRING:
            return ByteString(data_.identifier.byteString);
        default:
            return {};
        }
    }
};

}  // namespace opcua
