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

UA_DataTypeMember TypeHandler<UA_DataTypeMember>::copy(const UA_DataTypeMember& native) {
    return detail::copy(native);
}

/// NOLINTNEXTLINE(*param-not-moved)
UA_DataTypeMember TypeHandler<UA_DataTypeMember>::move(UA_DataTypeMember&& native) noexcept {
    return std::exchange(native, {});
}

void TypeHandler<UA_DataTypeMember>::clear(UA_DataTypeMember& native) noexcept {
    detail::clear(native);
}

UA_DataType TypeHandler<UA_DataType>::copy(const UA_DataType& native) {
    return detail::copy(native);
}

/// NOLINTNEXTLINE(*param-not-moved)
UA_DataType TypeHandler<UA_DataType>::move(UA_DataType&& native) noexcept {
    return std::exchange(native, {});
}

void TypeHandler<UA_DataType>::clear(UA_DataType& native) noexcept {
    detail::clear(native);
}

void DataType::setMembers(Span<const DataTypeMember> members) {
    assert(members.size() < (1U << 8U));
    detail::clearMembers(native());
    detail::copyMembers(asNative(members.data()), members.size(), native());
}

const UA_DataType* findDataType(const NodeId& id) noexcept {
    // UA_TYPES array is sorted by typeId -> use binary search
    const Span types{UA_TYPES, UA_TYPES_COUNT};  // NOLINT(*decay)
    const auto* it = std::lower_bound(
        types.begin(), types.end(), id, [](const UA_DataType& type, const NodeId& value) {
            return type.typeId < value;
        }
    );
    if (it != types.end() && it->typeId == id) {
        return it;
    }
    return nullptr;
}

const UA_DataType* findDataType(const NodeId& id, const UA_DataTypeArray* custom) noexcept {
    const auto* type = findDataType(id);
    if (type != nullptr) {
        return type;
    }
    while (custom != nullptr) {
        const Span types{custom->types, custom->typesSize};
        const auto* it = std::find_if(types.begin(), types.end(), [&](const auto& dt) {
            return dt.typeId == id;
        });
        if (it != types.end()) {
            return it;
        }
        custom = custom->next;
    }
    return nullptr;
}

}  // namespace opcua
