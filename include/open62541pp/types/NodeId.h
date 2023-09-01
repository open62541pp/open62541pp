#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>  // hash
#include <string>
#include <string_view>
#include <variant>

#include "open62541pp/Common.h"  // Type
#include "open62541pp/NodeIds.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/helper.h"
#include "open62541pp/open62541.h"
#include "open62541pp/types/Builtin.h"

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
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit constructors

    /// Create NodeId with numeric identifier.
    NodeId(uint16_t namespaceIndex, uint32_t identifier) noexcept;

    /// Create NodeId with String identifier from standard strings.
    NodeId(uint16_t namespaceIndex, std::string_view identifier);

    /// Create NodeId with String identifier from String wrapper class.
    NodeId(uint16_t namespaceIndex, const String& identifier);

    /// Create NodeId with Guid identifier.
    NodeId(uint16_t namespaceIndex, const Guid& identifier);

    /// Create NodeId with ByteString identifier.
    NodeId(uint16_t namespaceIndex, const ByteString& identifier);

    /// Create NodeId from Type (type id).
    NodeId(Type type) noexcept  // NOLINT, implicit wanted
        : NodeId(detail::getUaDataType(type).typeId) {}

    /// Create NodeId from DataTypeId.
    NodeId(DataTypeId id) noexcept  // NOLINT, implicit wanted
        : NodeId(0, static_cast<uint32_t>(id)) {}

    /// Create NodeId from ReferenceTypeId.
    NodeId(ReferenceTypeId id) noexcept  // NOLINT, implicit wanted
        : NodeId(0, static_cast<uint32_t>(id)) {}

    /// Create NodeId from ObjectTypeId.
    NodeId(ObjectTypeId id) noexcept  // NOLINT, implicit wanted
        : NodeId(0, static_cast<uint32_t>(id)) {}

    /// Create NodeId from VariableTypeId.
    NodeId(VariableTypeId id) noexcept  // NOLINT, implicit wanted
        : NodeId(0, static_cast<uint32_t>(id)) {}

    /// Create NodeId from ObjectId.
    NodeId(ObjectId id) noexcept  // NOLINT, implicit wanted
        : NodeId(0, static_cast<uint32_t>(id)) {}

    /// Create NodeId from VariableId.
    NodeId(VariableId id) noexcept  // NOLINT, implicit wanted
        : NodeId(0, static_cast<uint32_t>(id)) {}

    /// Create NodeId from MethodId.
    NodeId(MethodId id) noexcept  // NOLINT, implicit wanted
        : NodeId(0, static_cast<uint32_t>(id)) {}

    bool isNull() const noexcept;

    uint32_t hash() const noexcept;

    uint16_t getNamespaceIndex() const noexcept;

    NodeIdType getIdentifierType() const noexcept;

    /// Get identifier variant.
    std::variant<uint32_t, String, Guid, ByteString> getIdentifier() const;

    /// Get identifier by template type.
    template <typename T>
    auto getIdentifierAs() const {
        return std::get<T>(getIdentifier());
    }

    /// Get identifier by NodeIdType enum.
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

    /// Encode NodeId as a string like `ns=1;s=SomeNode`.
    /// @see https://reference.opcfoundation.org/Core/Part6/v105/docs/5.3.1.10
    std::string toString() const;
};

/**
 * UA_ExpandedNodeId wrapper class.
 * @ingroup TypeWrapper
 */
class ExpandedNodeId : public TypeWrapper<UA_ExpandedNodeId, UA_TYPES_EXPANDEDNODEID> {
public:
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit constructors

    explicit ExpandedNodeId(const NodeId& id);
    ExpandedNodeId(const NodeId& id, std::string_view namespaceUri, uint32_t serverIndex);

    bool isLocal() const noexcept;

    uint32_t hash() const noexcept;

    NodeId& getNodeId() noexcept;
    const NodeId& getNodeId() const noexcept;

    std::string_view getNamespaceUri() const;

    uint32_t getServerIndex() const noexcept;

    /// Encode ExpandedNodeId as a string like `svr=1;nsu=http://test.org/UA/Data/;ns=2;i=10157`.
    /// @see https://reference.opcfoundation.org/Core/Part6/v105/docs/5.3.1.11
    std::string toString() const;
};

}  // namespace opcua

/* ---------------------------------- std::hash specializations --------------------------------- */

template <>
struct std::hash<opcua::NodeId> {
    std::size_t operator()(const opcua::NodeId& id) const noexcept {
        return id.hash();
    }
};

template <>
struct std::hash<opcua::ExpandedNodeId> {
    std::size_t operator()(const opcua::ExpandedNodeId& id) const noexcept {
        return id.hash();
    }
};
