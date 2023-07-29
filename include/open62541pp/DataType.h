#pragma once

#include <cstdint>
#include <vector>

#include "open62541pp/Common.h"
#include "open62541pp/Config.h"
#include "open62541pp/open62541.h"

namespace opcua {

// forward declare
class NodeId;

using DataTypeMember = UA_DataTypeMember;

/**
 * UA_DataType wrapper class.
 */
class DataType {
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

    /// Implicit conversion to UA_DataType.
    constexpr operator UA_DataType&() noexcept {  // NOLINT
        return data_;
    }

    /// Implicit conversion to UA_DataType.
    constexpr operator const UA_DataType&() const noexcept {  // NOLINT
        return data_;
    }

    const char* getTypeName() const noexcept;
    void setTypeName(const char* typeName) noexcept;

    const NodeId& getTypeId() const noexcept;
    void setTypeId(const NodeId& typeId);
    void setTypeId(NodeId&& typeId) noexcept;

    const NodeId& getBinaryEncodingId() const noexcept;
    void setBinaryEncodingId(const NodeId& binaryEncodingId);
    void setBinaryEncodingId(NodeId&& binaryEncodingId) noexcept;

    uint16_t getMemSize() const noexcept;
    void setMemSize(uint16_t memSize) noexcept;

    uint8_t getTypeKind() const noexcept;
    void setTypeKind(uint8_t typeKind) noexcept;

    bool getPointerFree() const noexcept;
    void setPointerFree(bool pointerFree) noexcept;

    bool getOverlayable() const noexcept;
    void setOverlayable(bool overlayable) noexcept;

    uint8_t getMembersSize() const noexcept;
    std::vector<DataTypeMember> getMembers() const;
    void setMembers(const std::vector<DataTypeMember>& members);

    constexpr UA_DataType* handle() noexcept {
        return &data_;
    }

    constexpr const UA_DataType* handle() const noexcept {
        return &data_;
    }

private:
    UA_DataType data_{};
};

bool operator==(const UA_DataTypeMember& lhs, const UA_DataTypeMember& rhs) noexcept;
bool operator!=(const UA_DataTypeMember& lhs, const UA_DataTypeMember& rhs) noexcept;
bool operator==(const UA_DataType& lhs, const UA_DataType& rhs) noexcept;
bool operator!=(const UA_DataType& lhs, const UA_DataType& rhs) noexcept;
bool operator==(const DataType& lhs, const DataType& rhs) noexcept;
bool operator!=(const DataType& lhs, const DataType& rhs) noexcept;

/* ------------------------------------------- Helper ------------------------------------------- */

namespace detail {

[[nodiscard]] constexpr DataTypeMember createDataTypeMember(
    [[maybe_unused]] const char* memberName,
    const UA_DataType& memberType,
    uint8_t padding,
    bool isArray,
    bool isOptional
) {
    return {
#ifdef UA_ENABLE_TYPEDESCRIPTION
        memberName,
#endif
        &memberType,
        padding,
        static_cast<uint8_t>(isArray),
        static_cast<uint8_t>(isOptional),
    };
}

}  // namespace detail

}  // namespace opcua
