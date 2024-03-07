#include "open62541pp/DataType.h"

#include <algorithm>  // copy
#include <cassert>
#include <cstring>
#include <utility>  // exchange, move, swap

#include "open62541pp/Config.h"

namespace opcua {

static void clearMembers(UA_DataType& native) noexcept {
    delete[] native.members;  // NOLINT
    native.members = nullptr;
    native.membersSize = 0;
}

static void clear(UA_DataType& native) noexcept {
    clearMembers(native);
}

static void copyMembers(const DataTypeMember* members, size_t membersSize, UA_DataType& dst) {
    dst.members = new DataTypeMember[membersSize];  // NOLINT
    dst.membersSize = membersSize;
    std::copy(
        members,
        members + membersSize,  // NOLINT
        dst.members
    );
}

[[nodiscard]] static UA_DataType copy(const UA_DataType& other) {
    UA_DataType result{other};
    copyMembers(other.members, other.membersSize, result);
    return result;
}

DataType::DataType(const UA_DataType& native)
    : Wrapper(copy(native)) {}

DataType::DataType(UA_DataType&& native)
    : Wrapper(std::move(native)) {}

DataType::DataType(TypeIndex typeIndex)
    : DataType(UA_TYPES[typeIndex]) {  // NOLINT
    assert(typeIndex < UA_TYPES_COUNT);
}

DataType::~DataType() {
    clear(native());
}

DataType::DataType(const DataType& other)
    : Wrapper(copy(other.native())) {}

DataType::DataType(DataType&& other) noexcept
    : Wrapper(std::exchange(other.native(), {})) {}

DataType& DataType::operator=(const DataType& other) {
    if (this != &other) {
        clear(native());
        native() = copy(other.native());
    }
    return *this;
}

DataType& DataType::operator=(DataType&& other) noexcept {
    if (this != &other) {
        clear(native());
        native() = std::exchange(other.native(), {});
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
    clearMembers(native());
    copyMembers(members.data(), members.size(), native());
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
