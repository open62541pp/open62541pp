#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>

#include "open62541pp/TypeWrapper.h"
#include "open62541pp/Types.h"
#include "open62541pp/open62541.h"

namespace opcua {

/**
 * NodeId types.
 * @see UA_NodeIdType
 */
enum class NodeIdType : uint8_t {
    Numeric = UA_NODEIDTYPE_NUMERIC,
    String = UA_NODEIDTYPE_STRING,
    Guid = UA_NODEIDTYPE_GUID,
    ByteString = UA_NODEIDTYPE_BYTESTRING,
};

/**
 * UA_NodeId wrapper class.
 * @ingroup TypeWrapper
 */
class NodeId : public TypeWrapper<UA_NodeId, UA_TYPES_NODEID> {
public:
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    /// Create NodeId with numeric identifier
    NodeId(uint32_t identifier, uint16_t namespaceIndex);

    /// Create NodeId with String identifier from standard strings
    NodeId(std::string_view identifier, uint16_t namespaceIndex);

    /// Create NodeId with String identifier from String wrapper class
    NodeId(const String& identifier, uint16_t namespaceIndex);

    /// Create NodeId with Guid identifier
    NodeId(const Guid& identifier, uint16_t namespaceIndex);

    /// Create NodeId with ByteString identifier
    NodeId(const ByteString& identifier, uint16_t namespaceIndex);

    uint32_t hash() const;

    uint16_t getNamespaceIndex() const noexcept;

    NodeIdType getIdentifierType() const noexcept;

    /// Get identifier variant
    std::variant<uint32_t, String, Guid, ByteString> getIdentifier() const;

    /// Get identifier by template type
    template <typename T>
    auto getIdentifierAs() const {
        return std::get<T>(getIdentifier());
    }

    /// Get identifier by NodeIdType enum
    template <NodeIdType E>
    auto getIdentifierAs() const {
        if constexpr (E == NodeIdType::Numeric) {
            return getIdentifierAs<uint32_t>();
        }
        if constexpr (E == NodeIdType::String) {
            return getIdentifierAs<String>();
        }
        if constexpr (E == NodeIdType::Guid) {
            return getIdentifierAs<Guid>();
        }
        if constexpr (E == NodeIdType::ByteString) {
            return getIdentifierAs<ByteString>();
        }
    }
};

/**
 * UA_ExpandedNodeId wrapper class.
 * @ingroup TypeWrapper
 */
class ExpandedNodeId : public TypeWrapper<UA_ExpandedNodeId, UA_TYPES_EXPANDEDNODEID> {
public:
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    ExpandedNodeId(const NodeId& id, std::string_view namespaceUri, uint32_t serverIndex);

    bool isLocal() const noexcept;

    NodeId getNodeId() const noexcept;

    std::string getNamespaceUri() const;
    std::string_view getNamespaceUriView() const;

    uint32_t getServerIndex() const noexcept;
};

}  // namespace opcua
