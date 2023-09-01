#include "open62541pp/DataType.h"

#include <algorithm>  // copy
#include <cassert>
#include <cstring>
#include <utility>  // move, swap

#include "open62541pp/Config.h"
#include "open62541pp/TypeWrapper.h"  // asWrapper
#include "open62541pp/overloads/comparison.h"
#include "open62541pp/types/NodeId.h"

namespace opcua {

[[maybe_unused]] inline static const char* emptyIfNullptr(const char* name) {
    return name == nullptr ? "" : name;
}

static void clearMembers(UA_DataType* native) {
    if (native != nullptr) {
        delete[] native->members;  // NOLINT
        native->members = nullptr;
        native->membersSize = 0;
    }
}

static void copyMembers(const DataTypeMember* members, size_t membersSize, UA_DataType* dst) {
    if (dst != nullptr) {
        dst->members = new DataTypeMember[membersSize];  // NOLINT
        dst->membersSize = membersSize;
        std::copy(
            members,
            members + membersSize,  // NOLINT
            dst->members
        );
    }
}

static void copy(const UA_DataType* src, UA_DataType* dst) {
    if (src != nullptr && dst != nullptr) {
        clearMembers(dst);
        *dst = *src;
        copyMembers(dst->members, dst->membersSize, dst);
    }
}

DataType::DataType(const UA_DataType& native) {
    copy(&native, handle());
}

DataType::DataType(UA_DataType&& native)
    : data_(native) {}

DataType::DataType(TypeIndex typeIndex)
    : DataType(UA_TYPES[typeIndex])  // NOLINT
{
    assert(typeIndex < UA_TYPES_COUNT);
}

DataType::~DataType() {
    clearMembers(handle());
}

DataType::DataType(const DataType& other) {
    copy(other.handle(), handle());
}

DataType::DataType(DataType&& other) noexcept {
    std::swap(data_, other.data_);
}

DataType& DataType::operator=(const DataType& other) {
    if (this != &other) {
        copy(other.handle(), handle());
    }
    return *this;
}

DataType& DataType::operator=(DataType&& other) noexcept {
    if (this != &other) {
        std::swap(data_, other.data_);
    }
    return *this;
}

// NOLINTNEXTLINE
const char* DataType::getTypeName() const noexcept {
#ifdef UA_ENABLE_TYPEDESCRIPTION
    return handle()->typeName;
#else
    return nullptr;
#endif
}

void DataType::setTypeName([[maybe_unused]] const char* typeName) noexcept {
#ifdef UA_ENABLE_TYPEDESCRIPTION
    handle()->typeName = typeName;
#endif
}

NodeId DataType::getTypeId() const noexcept {
    return NodeId(handle()->typeId);  // NOLINT
}

void DataType::setTypeId(const NodeId& typeId) {
    asWrapper<NodeId>(handle()->typeId) = typeId;
}

void DataType::setTypeId(NodeId&& typeId) noexcept {
    asWrapper<NodeId>(handle()->typeId) = std::move(typeId);
}

NodeId DataType::getBinaryEncodingId() const noexcept {
#if UAPP_OPEN62541_VER_GE(1, 2)
    return NodeId(handle()->binaryEncodingId);  // NOLINT
#else
    return NodeId(handle()->typeId.namespaceIndex, handle()->binaryEncodingId);  // NOLINT
#endif
}

void DataType::setBinaryEncodingId(const NodeId& binaryEncodingId) {
#if UAPP_OPEN62541_VER_GE(1, 2)
    asWrapper<NodeId>(handle()->binaryEncodingId) = binaryEncodingId;
#else
    handle()->binaryEncodingId = binaryEncodingId.getIdentifierAs<uint32_t>();
#endif
}

void DataType::setBinaryEncodingId(NodeId&& binaryEncodingId) {
#if UAPP_OPEN62541_VER_GE(1, 2)
    asWrapper<NodeId>(handle()->binaryEncodingId) = std::move(binaryEncodingId);
#else
    handle()->binaryEncodingId = binaryEncodingId.getIdentifierAs<uint32_t>();
#endif
}

uint16_t DataType::getMemSize() const noexcept {
    return handle()->memSize;
}

void DataType::setMemSize(uint16_t memSize) noexcept {
    handle()->memSize = memSize;
}

uint8_t DataType::getTypeKind() const noexcept {
    return handle()->typeKind;
}

void DataType::setTypeKind(uint8_t typeKind) noexcept {
    assert(typeKind < UA_DATATYPEKINDS);
    handle()->typeKind = typeKind;
}

bool DataType::getPointerFree() const noexcept {
    return handle()->pointerFree;
}

void DataType::setPointerFree(bool pointerFree) noexcept {
    handle()->pointerFree = pointerFree;
}

bool DataType::getOverlayable() const noexcept {
    return handle()->overlayable;
}

void DataType::setOverlayable(bool overlayable) noexcept {
    handle()->overlayable = overlayable;
}

Span<const DataTypeMember> DataType::getMembers() const noexcept {
    return {handle()->members, handle()->membersSize};
}

void DataType::setMembers(Span<const DataTypeMember> members) {
    assert(members.size() < (1U << 8U));
    clearMembers(handle());
    copyMembers(members.data(), members.size(), handle());
}

bool operator==(const UA_DataTypeMember& lhs, const UA_DataTypeMember& rhs) noexcept {
#if UAPP_OPEN62541_VER_GE(1, 3)
    if (lhs.memberType != rhs.memberType) {
        return false;
    }
#else
    if ((lhs.memberTypeIndex != rhs.memberTypeIndex) && (lhs.namespaceZero != rhs.namespaceZero)) {
        return false;
    }
#endif
    if (lhs.padding != rhs.padding) {
        return false;
    }
    if (lhs.isArray != rhs.isArray) {
        return false;
    }
#if UAPP_OPEN62541_VER_GE(1, 1)
    if (lhs.isOptional != rhs.isOptional) {
        return false;
    }
#endif
#ifdef UA_ENABLE_TYPEDESCRIPTION
    if (std::strcmp(emptyIfNullptr(lhs.memberName), emptyIfNullptr(rhs.memberName)) != 0) {
        return false;
    }
#endif
    return true;
}

bool operator!=(const UA_DataTypeMember& lhs, const UA_DataTypeMember& rhs) noexcept {
    return !(lhs == rhs);
}

bool operator==(const UA_DataType& lhs, const UA_DataType& rhs) noexcept {
    if (lhs.typeId != rhs.typeId) {
        return false;
    }
    if (lhs.binaryEncodingId != rhs.binaryEncodingId) {
        return false;
    }
    if (lhs.memSize != rhs.memSize) {
        return false;
    }
    if (lhs.typeKind != rhs.typeKind) {
        return false;
    }
    if (lhs.pointerFree != rhs.pointerFree) {
        return false;
    }
    if (lhs.overlayable != rhs.overlayable) {
        return false;
    }
    if (lhs.membersSize != rhs.membersSize) {
        return false;
    }
    for (size_t i = 0; i < lhs.membersSize; ++i) {
        if (lhs.members[i] != rhs.members[i]) {  // NOLINT
            return false;
        }
    }
#ifdef UA_ENABLE_TYPEDESCRIPTION
    if (std::strcmp(emptyIfNullptr(lhs.typeName), emptyIfNullptr(rhs.typeName)) != 0) {
        return false;  // NOLINT
    }
#endif
    return true;
}

bool operator!=(const UA_DataType& lhs, const UA_DataType& rhs) noexcept {
    return !(lhs == rhs);
}

bool operator==(const DataType& lhs, const DataType& rhs) noexcept {
    return (*lhs.handle() == *rhs.handle());
}

bool operator!=(const DataType& lhs, const DataType& rhs) noexcept {
    return !(lhs == rhs);
}

namespace detail {

DataTypeMember createDataTypeMember(
    [[maybe_unused]] const char* memberName,
    const UA_DataType& memberType,
    uint8_t padding,
    bool isArray,
    [[maybe_unused]] bool isOptional
) noexcept {
    DataTypeMember result{};
#ifdef UA_ENABLE_TYPEDESCRIPTION
    result.memberName = memberName;
#endif
#if UAPP_OPEN62541_VER_GE(1, 3)
    result.memberType = &memberType;
#else
    result.memberTypeIndex = memberType.typeIndex;
    result.namespaceZero = memberType.typeId.namespaceIndex == 0;
#endif
    result.padding = padding;
    result.isArray = isArray;  // NOLINT
#if UAPP_OPEN62541_VER_GE(1, 1)
    result.isOptional = isOptional;  // NOLINT
#endif
    return result;
}

UA_DataType createDataType(
    [[maybe_unused]] const char* typeName,
    UA_NodeId typeId,
    UA_NodeId binaryEncodingId,
    uint16_t memSize,
    uint8_t typeKind,
    bool pointerFree,
    bool overlayable,
    uint32_t membersSize,
    DataTypeMember* members
) noexcept {
    UA_DataType result{};
#ifdef UA_ENABLE_TYPEDESCRIPTION
    result.typeName = typeName;
#endif
    result.typeId = typeId;
#if UAPP_OPEN62541_VER_GE(1, 2)
    result.binaryEncodingId = binaryEncodingId;
#else
    assert(binaryEncodingId.identifierType == UA_NODEIDTYPE_NUMERIC);
    result.binaryEncodingId = binaryEncodingId.identifier.numeric;  // NOLINT
#endif
    result.memSize = memSize;
    result.typeKind = typeKind;
    result.pointerFree = pointerFree;
    result.overlayable = overlayable;
    result.membersSize = membersSize;
    result.members = members;
    return result;
}

}  // namespace detail

}  // namespace opcua
