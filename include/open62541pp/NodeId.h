#pragma once

#include <string_view>
#include <variant>

#include "open62541pp/TypeWrapper.h"
#include "open62541pp/open62541.h"

namespace opcua {

/**
 * UA_NodeId wrapper class.
 */
class NodeId : public TypeWrapper<UA_NodeId> {
public:
    using BaseClass::BaseClass;  // inherit contructors

    /// Create NodeId with numeric identifier
    NodeId(UA_UInt32 identifier, UA_UInt16 namespaceIndex);

    /// Create NodeId with String identifier from standard strings
    NodeId(std::string_view identifier, UA_UInt16 namespaceIndex);

    /// Create NodeId with String identifier from String wrapper class
    NodeId(const String& identifier, UA_UInt16 namespaceIndex);

    /// Create NodeId with Guid identifier
    NodeId(const Guid& identifier, UA_UInt16 namespaceIndex);

    /// Create NodeId with ByteString identifier
    NodeId(const ByteString& identifier, UA_UInt16 namespaceIndex);

    bool operator==(const NodeId& other) const noexcept;
    bool operator!=(const NodeId& other) const noexcept;
    bool operator<(const NodeId& other) const noexcept;
    bool operator>(const NodeId& other) const noexcept;

    UA_UInt32 hash() const;

    UA_UInt16 getNamespaceIndex() const noexcept;

    UA_NodeIdType getIdentifierType() const noexcept;

    std::variant<UA_UInt32, String, Guid, ByteString> getIdentifier() const;
};

}  // namespace opcua
