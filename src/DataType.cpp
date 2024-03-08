#include "open62541pp/DataType.h"

#include <algorithm>  // copy_n
#include <cassert>
#include <utility>  // exchange

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
    std::copy_n(
        members,
        membersSize,
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
    : Wrapper(native) {}

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
