#include "open62541pp/datatype.hpp"

#include <algorithm>  // copy_n
#include <cassert>
#include <new>  // placement new
#include <utility>  // exchange

#include "open62541pp/detail/types_handling.hpp"

namespace opcua {

namespace detail {

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

void clear(UA_DataTypeArray& native) noexcept {
#if UAPP_OPEN62541_VER_GE(1, 4)
    const bool cleanup = native.cleanup;
#else
    const bool cleanup = true;
#endif
    if (cleanup) {
        std::for_each_n(
            const_cast<UA_DataType*>(native.types),  // NOLINT
            native.typesSize,
            [](auto& type) { clear(type); }
        );
    }
    deallocateArray(const_cast<UA_DataType*>(native.types));  // NOLINT
    native.types = nullptr;
}

void deallocate(const UA_DataTypeArray* head) noexcept {
    while (head != nullptr) {
        const auto* next = head->next;
        auto* item = const_cast<UA_DataTypeArray*>(head);  // NOLINT(*const-cast)
        detail::clear(*item);
        detail::deallocate(item);
        head = next;
    }
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
    std::transform(
        members,
        members + membersSize,  // NOLINT(*pointer-arithmetic)
        dst.members,
        [](const auto& member) { return copy(member); }
    );
}

UA_DataType copy(const UA_DataType& src) {
    UA_DataType dst{src};
#if UAPP_HAS_TYPEDESCRIPTION
    dst.typeName = detail::allocCString(src.typeName);
#endif
    copyMembers(src.members, src.membersSize, dst);
    return dst;
}

UA_DataType* copyArray(const UA_DataType* src, size_t size) {
    auto* dst = allocateArray<UA_DataType>(size);
    if (src == nullptr) {
        return dst;
    }
    std::transform(src, src + size, dst, [](const auto& type) { return copy(type); });  // NOLINT
    return dst;
}

void addDataTypes(const UA_DataTypeArray*& head, Span<const DataType> types) {
    auto* item = allocate<UA_DataTypeArray>();
    new (item) UA_DataTypeArray{
        head,  // next
        types.size(),
        copyArray(asNative(types.data()), types.size()),
#if UAPP_OPEN62541_VER_GE(1, 4)
        true,  // cleanup
#endif
    };
    head = item;
}

}  // namespace detail

DataTypeMember::DataTypeMember(const UA_DataTypeMember& native)
    : Wrapper{detail::copy(native)} {}

DataTypeMember::DataTypeMember(UA_DataTypeMember&& native) noexcept  // NOLINT
    : Wrapper{std::exchange(native, {})} {}

DataTypeMember::~DataTypeMember() {
    detail::clear(native());
}

DataTypeMember::DataTypeMember(const DataTypeMember& other)
    : Wrapper{detail::copy(other.native())} {}

DataTypeMember::DataTypeMember(DataTypeMember&& other) noexcept
    : Wrapper{std::exchange(other.native(), {})} {}

DataTypeMember& DataTypeMember::operator=(const DataTypeMember& other) {
    if (this != &other) {
        detail::clear(native());
        native() = detail::copy(other.native());
    }
    return *this;
}

DataTypeMember& DataTypeMember::operator=(DataTypeMember&& other) noexcept {
    if (this != &other) {
        detail::clear(native());
        native() = std::exchange(other.native(), {});
    }
    return *this;
}

DataType::DataType(const UA_DataType& native)
    : Wrapper{detail::copy(native)} {}

DataType::DataType(UA_DataType&& native) noexcept  // NOLINT
    : Wrapper{std::exchange(native, {})} {}

DataType::DataType(TypeIndex typeIndex)
    : DataType{UA_TYPES[typeIndex]} {  // NOLINT
    assert(typeIndex < UA_TYPES_COUNT);
}

DataType::~DataType() {
    detail::clear(native());
}

DataType::DataType(const DataType& other)
    : Wrapper{detail::copy(other.native())} {}

DataType::DataType(DataType&& other) noexcept
    : Wrapper{std::exchange(other.native(), {})} {}

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
    detail::copyMembers(asNative(members.data()), members.size(), native());
}

}  // namespace opcua
