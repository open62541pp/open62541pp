#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>  // hash
#include <string>
#include <string_view>
#include <utility>  // move
#include <variant>

#include "open62541pp/Common.h"  // NamespaceIndex, Namespace, Type
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/detail/string_utils.h"  // detail::allocNativeString
#include "open62541pp/types/Builtin.h"

namespace opcua {

namespace detail {
template <typename T, typename = void>
struct IsNodeIdEnum : std::false_type {};

template <typename T>
struct IsNodeIdEnum<T, std::void_t<decltype(getNamespace(std::declval<T>()))>> : std::true_type {};

}  // namespace detail

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
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/8.2
 * @ingroup Wrapper
 */
class NodeId : public TypeWrapper<UA_NodeId, UA_TYPES_NODEID> {
public:
    using TypeWrapper::TypeWrapper;  // inherit constructors

    /// Create NodeId with numeric identifier.
    NodeId(NamespaceIndex namespaceIndex, uint32_t identifier) noexcept {
        handle()->namespaceIndex = namespaceIndex;
        handle()->identifierType = UA_NODEIDTYPE_NUMERIC;
        handle()->identifier.numeric = identifier;  // NOLINT
    }

    /// Create NodeId with String identifier from standard strings.
    NodeId(NamespaceIndex namespaceIndex, std::string_view identifier) {
        handle()->namespaceIndex = namespaceIndex;
        handle()->identifierType = UA_NODEIDTYPE_STRING;
        handle()->identifier.string = detail::allocNativeString(identifier);  // NOLINT
    }

    /// Create NodeId with String identifier from String wrapper class.
    NodeId(NamespaceIndex namespaceIndex, String identifier) noexcept {
        handle()->namespaceIndex = namespaceIndex;
        handle()->identifierType = UA_NODEIDTYPE_STRING;
        handle()->identifier.string = std::exchange(asNative(identifier), {});  // NOLINT
    }

    /// Create NodeId with Guid identifier.
    NodeId(NamespaceIndex namespaceIndex, Guid identifier) noexcept {
        handle()->namespaceIndex = namespaceIndex;
        handle()->identifierType = UA_NODEIDTYPE_GUID;
        handle()->identifier.guid = identifier;  // NOLINT
    }

    /// Create NodeId with ByteString identifier.
    NodeId(NamespaceIndex namespaceIndex, ByteString identifier) noexcept {
        handle()->namespaceIndex = namespaceIndex;
        handle()->identifierType = UA_NODEIDTYPE_BYTESTRING;
        handle()->identifier.byteString = std::exchange(asNative(identifier), {});  // NOLINT
    }

    /// Create NodeId from enum class with numeric identifiers like `opcua::ObjectId`.
    /// The namespace is retrieved by calling e.g. `getNamespace(opcua::ObjectId)`.
    /// Make sure to provide an overload for custom enum types.
    template <typename T, typename = std::enable_if_t<detail::IsNodeIdEnum<T>::value>>
    NodeId(T identifier) noexcept  // NOLINT, implicit wanted
        : NodeId(getNamespace(identifier).index, static_cast<uint32_t>(identifier)) {}

    bool isNull() const noexcept {
        return UA_NodeId_isNull(handle());
    }

    uint32_t hash() const noexcept {
        return UA_NodeId_hash(handle());
    }

    NamespaceIndex getNamespaceIndex() const noexcept {
        return handle()->namespaceIndex;
    }

    NodeIdType getIdentifierType() const noexcept {
        return static_cast<NodeIdType>(handle()->identifierType);
    }

    /// Get identifier variant.
    std::variant<uint32_t, String, Guid, ByteString> getIdentifier() const {
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

inline bool operator==(const UA_NodeId& lhs, const UA_NodeId& rhs) noexcept {
    return UA_NodeId_equal(&lhs, &rhs);
}

inline bool operator!=(const UA_NodeId& lhs, const UA_NodeId& rhs) noexcept {
    return !(lhs == rhs);
}

inline bool operator<(const UA_NodeId& lhs, const UA_NodeId& rhs) noexcept {
    return UA_NodeId_order(&lhs, &rhs) == UA_ORDER_LESS;
}

inline bool operator>(const UA_NodeId& lhs, const UA_NodeId& rhs) noexcept {
    return UA_NodeId_order(&lhs, &rhs) == UA_ORDER_MORE;
}

inline bool operator<=(const UA_NodeId& lhs, const UA_NodeId& rhs) noexcept {
    return (lhs < rhs) || (lhs == rhs);
}

inline bool operator>=(const UA_NodeId& lhs, const UA_NodeId& rhs) noexcept {
    return (lhs > rhs) || (lhs == rhs);
}

/**
 * UA_ExpandedNodeId wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.16
 * @ingroup Wrapper
 */
class ExpandedNodeId : public TypeWrapper<UA_ExpandedNodeId, UA_TYPES_EXPANDEDNODEID> {
public:
    using TypeWrapper::TypeWrapper;  // inherit constructors

    explicit ExpandedNodeId(NodeId id) noexcept {
        asWrapper<NodeId>(handle()->nodeId) = std::move(id);
    }

    ExpandedNodeId(NodeId id, std::string_view namespaceUri, uint32_t serverIndex) {
        asWrapper<NodeId>(handle()->nodeId) = std::move(id);
        handle()->namespaceUri = detail::allocNativeString(namespaceUri);
        handle()->serverIndex = serverIndex;
    }

    bool isLocal() const noexcept {
        return handle()->serverIndex == 0;
    }

    uint32_t hash() const noexcept {
        return UA_ExpandedNodeId_hash(handle());
    }

    NodeId& getNodeId() noexcept {
        return asWrapper<NodeId>(handle()->nodeId);
    }

    const NodeId& getNodeId() const noexcept {
        return asWrapper<NodeId>(handle()->nodeId);
    }

    std::string_view getNamespaceUri() const {
        return detail::toStringView(handle()->namespaceUri);
    }

    uint32_t getServerIndex() const noexcept {
        return handle()->serverIndex;
    }

    /// Encode ExpandedNodeId as a string like `svr=1;nsu=http://test.org/UA/Data/;ns=2;i=10157`.
    /// @see https://reference.opcfoundation.org/Core/Part6/v105/docs/5.3.1.11
    std::string toString() const;
};

inline bool operator==(const UA_ExpandedNodeId& lhs, const UA_ExpandedNodeId& rhs) noexcept {
    return UA_ExpandedNodeId_equal(&lhs, &rhs);
}

inline bool operator!=(const UA_ExpandedNodeId& lhs, const UA_ExpandedNodeId& rhs) noexcept {
    return !(lhs == rhs);
}

inline bool operator<(const UA_ExpandedNodeId& lhs, const UA_ExpandedNodeId& rhs) noexcept {
    return UA_ExpandedNodeId_order(&lhs, &rhs) == UA_ORDER_LESS;
}

inline bool operator>(const UA_ExpandedNodeId& lhs, const UA_ExpandedNodeId& rhs) noexcept {
    return UA_ExpandedNodeId_order(&lhs, &rhs) == UA_ORDER_MORE;
}

inline bool operator<=(const UA_ExpandedNodeId& lhs, const UA_ExpandedNodeId& rhs) noexcept {
    return (lhs < rhs) || (lhs == rhs);
}

inline bool operator>=(const UA_ExpandedNodeId& lhs, const UA_ExpandedNodeId& rhs) noexcept {
    return (lhs > rhs) || (lhs == rhs);
}

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
