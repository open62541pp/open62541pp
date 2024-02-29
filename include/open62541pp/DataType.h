#pragma once

#include <cstdint>

#include "open62541pp/Common.h"
#include "open62541pp/Span.h"
#include "open62541pp/Wrapper.h"
#include "open62541pp/detail/open62541/common.h"

namespace opcua {

// forward declare
class NodeId;

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

    const char* getTypeName() const noexcept;
    void setTypeName(const char* typeName) noexcept;

    NodeId getTypeId() const noexcept;
    void setTypeId(const NodeId& typeId);
    void setTypeId(NodeId&& typeId) noexcept;

    NodeId getBinaryEncodingId() const noexcept;
    void setBinaryEncodingId(const NodeId& binaryEncodingId);
    void setBinaryEncodingId(NodeId&& binaryEncodingId);

    uint16_t getMemSize() const noexcept;
    void setMemSize(uint16_t memSize) noexcept;

    uint8_t getTypeKind() const noexcept;
    void setTypeKind(uint8_t typeKind) noexcept;

    bool getPointerFree() const noexcept;
    void setPointerFree(bool pointerFree) noexcept;

    bool getOverlayable() const noexcept;
    void setOverlayable(bool overlayable) noexcept;

    Span<const DataTypeMember> getMembers() const noexcept;
    void setMembers(Span<const DataTypeMember> members);
};

bool operator==(const UA_DataTypeMember& lhs, const UA_DataTypeMember& rhs) noexcept;
bool operator!=(const UA_DataTypeMember& lhs, const UA_DataTypeMember& rhs) noexcept;
bool operator==(const UA_DataType& lhs, const UA_DataType& rhs) noexcept;
bool operator!=(const UA_DataType& lhs, const UA_DataType& rhs) noexcept;
bool operator==(const DataType& lhs, const DataType& rhs) noexcept;
bool operator!=(const DataType& lhs, const DataType& rhs) noexcept;

/* ------------------------------------------- Helper ------------------------------------------- */

namespace detail {

[[nodiscard]] DataTypeMember createDataTypeMember(
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
