#include "open62541pp/datatype.hpp"

#include <algorithm>  // copy_n
#include <cassert>
#include <utility>  // exchange

#include "open62541pp/detail/types_handling.hpp"

namespace opcua {

namespace detail {

DataTypeMember createDataTypeMember(
    [[maybe_unused]] const char* memberName,
    const UA_DataType& memberType,
    uint8_t padding,
    bool isArray,
    [[maybe_unused]] bool isOptional
) noexcept {
    DataTypeMember result{};
#if UAPP_HAS_TYPEDESCRIPTION
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
#if UAPP_HAS_TYPEDESCRIPTION
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

UA_DataTypeArray createDataTypeArray(
    const UA_DataType* types, size_t typesSize, const UA_DataTypeArray* next
) noexcept {
    return UA_DataTypeArray{
        next,
        typesSize,
        types,
#if UAPP_OPEN62541_VER_GE(1, 4)
        false,  // cleanup
#endif
    };
}

void clear(UA_DataTypeMember& native) noexcept {
    detail::clear(native.memberName);
    native.memberName = nullptr;
}

static void clearMembers(UA_DataType& native) noexcept {
#if UAPP_HAS_TYPEDESCRIPTION
    std::for_each_n(native.members, native.membersSize, [](auto& member) { clear(member); });
#endif
    detail::deallocateArray(native.members);
    native.members = nullptr;
    native.membersSize = 0;
}

void clear(UA_DataType& native) noexcept {
#if UAPP_HAS_TYPEDESCRIPTION
    detail::clear(native.typeName);
#endif
    clearMembers(native);
    native = {};
}

UA_DataTypeMember copy(const UA_DataTypeMember& src) {
    UA_DataTypeMember dst{src};
#if UAPP_HAS_TYPEDESCRIPTION
    dst.memberName = detail::allocCString(src.memberName);
#endif
    return dst;
}

static void copyMembers(const UA_DataTypeMember* members, size_t membersSize, UA_DataType& dst) {
    dst.members = detail::allocateArray<UA_DataTypeMember>(membersSize);
    dst.membersSize = membersSize;
    std::transform(members, std::next(members, membersSize), dst.members, [](const auto& member) {
        return copy(member);
    });
}

UA_DataType copy(const UA_DataType& src) {
    UA_DataType dst{src};
#if UAPP_HAS_TYPEDESCRIPTION
    dst.typeName = detail::allocCString(src.typeName);
#endif
    copyMembers(src.members, src.membersSize, dst);
    return dst;
}

}  // namespace detail

DataType::DataType(const UA_DataType& native)
    : Wrapper(detail::copy(native)) {}

DataType::DataType(UA_DataType&& native)  // NOLINT
    : Wrapper(std::exchange(native, {})) {}

DataType::DataType(TypeIndex typeIndex)
    : DataType(UA_TYPES[typeIndex]) {  // NOLINT
    assert(typeIndex < UA_TYPES_COUNT);
}

DataType::~DataType() {
    detail::clear(native());
}

DataType::DataType(const DataType& other)
    : Wrapper(detail::copy(other.native())) {}

DataType::DataType(DataType&& other) noexcept
    : Wrapper(std::exchange(other.native(), {})) {}

DataType& DataType::operator=(const DataType& other) {
    if (this != &other) {
        detail::clear(native());
        native() = detail::copy(other.native());
    }
    return *this;
}

DataType& DataType::operator=(DataType&& other) noexcept {
    if (this != &other) {
        detail::clear(native());
        native() = std::exchange(other.native(), {});
    }
    return *this;
}

void DataType::setMembers(Span<const DataTypeMember> members) {
    assert(members.size() < (1U << 8U));
    detail::clearMembers(native());
    detail::copyMembers(members.data(), members.size(), native());
}

}  // namespace opcua
