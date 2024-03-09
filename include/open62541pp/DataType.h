#pragma once

#include <cstdint>
#include <utility>  // move

#include "open62541pp/Common.h"  // TypeIndex
#include "open62541pp/Config.h"
#include "open62541pp/Span.h"
#include "open62541pp/Wrapper.h"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/types/NodeId.h"

namespace opcua {

using DataTypeMember = UA_DataTypeMember;

/**
 * UA_DataType wrapper class.
 */
class DataType : public Wrapper<UA_DataType> {
public:
    constexpr DataType() = default;

    explicit DataType(const UA_DataType& native);
    explicit DataType(UA_DataType&& native);
    explicit DataType(TypeIndex typeIndex);

    ~DataType();

    DataType(const DataType& other);
    DataType(DataType&& other) noexcept;

    DataType& operator=(const DataType& other);
    DataType& operator=(DataType&& other) noexcept;

    const char* getTypeName() const noexcept {
#ifdef UA_ENABLE_TYPEDESCRIPTION
        return handle()->typeName;
#else
        return nullptr;
#endif
    }

    void setTypeName(const char* typeName) noexcept {
#ifdef UA_ENABLE_TYPEDESCRIPTION
        handle()->typeName = typeName;
#endif
    }

    NodeId getTypeId() const noexcept {
        return NodeId(handle()->typeId);  // NOLINT
    }

    void setTypeId(NodeId typeId) {
        asWrapper<NodeId>(handle()->typeId) = std::move(typeId);
    }

    NodeId getBinaryEncodingId() const noexcept {
#if UAPP_OPEN62541_VER_GE(1, 2)
        return NodeId(handle()->binaryEncodingId);  // NOLINT
#else
        return NodeId(handle()->typeId.namespaceIndex, handle()->binaryEncodingId);  // NOLINT
#endif
    }

    void setBinaryEncodingId(NodeId binaryEncodingId) {
#if UAPP_OPEN62541_VER_GE(1, 2)
        asWrapper<NodeId>(handle()->binaryEncodingId) = std::move(binaryEncodingId);
#else
        handle()->binaryEncodingId = binaryEncodingId.getIdentifierAs<uint32_t>();
#endif
    }

    uint16_t getMemSize() const noexcept {
        return handle()->memSize;
    }

    void setMemSize(uint16_t memSize) noexcept {
        handle()->memSize = memSize;
    }

    uint8_t getTypeKind() const noexcept {
        return handle()->typeKind;
    }

    void setTypeKind(uint8_t typeKind) noexcept {
        handle()->typeKind = typeKind;
    }

    bool getPointerFree() const noexcept {
        return handle()->pointerFree;
    }

    void setPointerFree(bool pointerFree) noexcept {
        handle()->pointerFree = pointerFree;
    }

    bool getOverlayable() const noexcept {
        return handle()->overlayable;
    }

    void setOverlayable(bool overlayable) noexcept {
        handle()->overlayable = overlayable;
    }

    Span<const DataTypeMember> getMembers() const noexcept {
        return {handle()->members, handle()->membersSize};
    }

    void setMembers(Span<const DataTypeMember> members);
};

inline bool operator==(const UA_DataType& lhs, const UA_DataType& rhs) noexcept {
    return lhs.typeId == rhs.typeId;
}

inline bool operator!=(const UA_DataType& lhs, const UA_DataType& rhs) noexcept {
    return !(lhs == rhs);
}

inline bool operator==(const UA_DataTypeMember& lhs, const UA_DataTypeMember& rhs) noexcept {
#if UAPP_OPEN62541_VER_GE(1, 3)
    if (lhs.memberType == nullptr || rhs.memberType == nullptr) {
        return false;
    }
    return (lhs.memberType == rhs.memberType) || (*lhs.memberType == *rhs.memberType);
#else
    return lhs.memberTypeIndex == rhs.memberTypeIndex;
#endif
}

inline bool operator!=(const UA_DataTypeMember& lhs, const UA_DataTypeMember& rhs) noexcept {
    return !(lhs == rhs);
}

/* ------------------------------------------- Helper ------------------------------------------- */

namespace detail {

[[nodiscard]] UA_DataTypeMember createDataTypeMember(
    const char* memberName,
    const UA_DataType& memberType,
    uint8_t padding,
    bool isArray,
    bool isOptional
) noexcept;

[[nodiscard]] UA_DataType createDataType(
    const char* typeName,
    UA_NodeId typeId,
    UA_NodeId binaryEncodingId,
    uint16_t memSize,
    uint8_t typeKind,
    bool pointerFree,
    bool overlayable,
    uint32_t membersSize,
    DataTypeMember* members
) noexcept;

}  // namespace detail

}  // namespace opcua
