#pragma once

#include <algorithm>  // transform
#include <cassert>
#include <cstdint>
#include <iterator>  // prev
#include <string_view>
#include <type_traits>
#include <utility>  // move
#include <vector>

#include "open62541pp/common.hpp"  // TypeIndex
#include "open62541pp/config.hpp"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/detail/string_utils.hpp"
#include "open62541pp/detail/traits.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/typeregistry.hpp"  // getDataType
#include "open62541pp/types.hpp"  // NodeId
#include "open62541pp/wrapper.hpp"

namespace opcua {

template <>
struct TypeHandler<UA_DataTypeMember> {
    static UA_DataTypeMember copy(const UA_DataTypeMember& native);
    static UA_DataTypeMember move(UA_DataTypeMember&& native) noexcept;
    static void clear(UA_DataTypeMember& native) noexcept;
};

template <>
struct TypeHandler<UA_DataType> {
    static UA_DataType copy(const UA_DataType& native);
    static UA_DataType move(UA_DataType&& native) noexcept;
    static void clear(UA_DataType& native) noexcept;
};

class DataTypeMember : public Wrapper<UA_DataTypeMember> {
public:
    using Wrapper::Wrapper;

    std::string_view memberName() const noexcept {
#if UAPP_HAS_TYPEDESCRIPTION
        return handle()->memberName;
#else
        return {};
#endif
    }

    void setMemberName([[maybe_unused]] std::string_view memberName) {
#if UAPP_HAS_TYPEDESCRIPTION
        detail::clear(handle()->memberName);
        handle()->memberName = detail::allocCString(memberName);
#endif
    }

    const UA_DataType* memberType() const noexcept {
#if UAPP_OPEN62541_VER_GE(1, 3)
        return handle()->memberType;
#else
        return nullptr;
#endif
    }

    void setMemberType(const UA_DataType* memberType) {
#if UAPP_OPEN62541_VER_GE(1, 3)
        handle()->memberType = memberType;
#else
        if (memberType != nullptr) {
            handle()->memberTypeIndex = memberType->typeIndex;
            handle()->namespaceZero = memberType->typeId.namespaceIndex == 0;
        }
#endif
    }

    uint8_t padding() const noexcept {
        return handle()->padding;
    }

    void setPadding(uint8_t padding) {
        handle()->padding = padding;
    }

    bool isArray() const noexcept {
        return handle()->isArray;
    }

    void setIsArray(bool isArray) {
        handle()->isArray = isArray;
    }

    bool isOptional() const noexcept {
#if UAPP_OPEN62541_VER_GE(1, 1)
        return handle()->isOptional;
#else
        return false;
#endif
    }

    void setIsOptional([[maybe_unused]] bool isOptional) {
#if UAPP_OPEN62541_VER_GE(1, 1)
        handle()->isOptional = isOptional;
#endif
    }
};

/**
 * UA_DataType wrapper class.
 * @ingroup Wrapper
 */
class DataType : public Wrapper<UA_DataType> {
public:
    using Wrapper::Wrapper;

    std::string_view typeName() const noexcept {
#if UAPP_HAS_TYPEDESCRIPTION
        return handle()->typeName;
#else
        return {};
#endif
    }

    void setTypeName([[maybe_unused]] std::string_view typeName) noexcept {
#if UAPP_HAS_TYPEDESCRIPTION
        detail::clear(handle()->typeName);
        handle()->typeName = detail::allocCString(typeName);
#endif
    }

    NodeId typeId() const noexcept {
        return NodeId{handle()->typeId};  // NOLINT
    }

    void setTypeId(NodeId typeId) {
        asWrapper<NodeId>(handle()->typeId) = std::move(typeId);
    }

    NodeId binaryEncodingId() const noexcept {
#if UAPP_OPEN62541_VER_GE(1, 2)
        return NodeId{handle()->binaryEncodingId};  // NOLINT
#else
        return NodeId{handle()->typeId.namespaceIndex, handle()->binaryEncodingId};  // NOLINT
#endif
    }

    void setBinaryEncodingId(NodeId binaryEncodingId) {
#if UAPP_OPEN62541_VER_GE(1, 2)
        asWrapper<NodeId>(handle()->binaryEncodingId) = std::move(binaryEncodingId);
#else
        handle()->binaryEncodingId = binaryEncodingId.identifier<uint32_t>();
#endif
    }

    uint16_t memSize() const noexcept {
        return handle()->memSize;
    }

    void setMemSize(uint16_t memSize) noexcept {
        handle()->memSize = memSize;
    }

    uint8_t typeKind() const noexcept {
        return handle()->typeKind;
    }

    void setTypeKind(uint8_t typeKind) noexcept {
        handle()->typeKind = typeKind;
    }

    bool pointerFree() const noexcept {
        return handle()->pointerFree;
    }

    void setPointerFree(bool pointerFree) noexcept {
        handle()->pointerFree = pointerFree;
    }

    bool overlayable() const noexcept {
        return handle()->overlayable;
    }

    void setOverlayable(bool overlayable) noexcept {
        handle()->overlayable = overlayable;
    }

    Span<const DataTypeMember> members() const noexcept {
        return {asWrapper<DataTypeMember>(handle()->members), handle()->membersSize};
    }

    void setMembers(Span<const DataTypeMember> members);
};

/// @relates DataType
inline bool operator==(const UA_DataType& lhs, const UA_DataType& rhs) noexcept {
    return lhs.typeId == rhs.typeId;
}

/// @relates DataType
inline bool operator!=(const UA_DataType& lhs, const UA_DataType& rhs) noexcept {
    return !(lhs == rhs);
}

/// @relates DataType
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

/// @relates DataType
inline bool operator!=(const UA_DataTypeMember& lhs, const UA_DataTypeMember& rhs) noexcept {
    return !(lhs == rhs);
}

/* --------------------------------------- DataTypeBuilder -------------------------------------- */

namespace detail {

template <auto memberPtr>
const UA_DataType& getMemberDataType() {
    using TMember = detail::MemberTypeT<decltype(memberPtr)>;
    return getDataType<std::remove_pointer_t<TMember>>();
}

// https://gist.github.com/graphitemaster/494f21190bb2c63c5516
template <typename T, typename TMember>
size_t offsetOfMember(TMember T::*member) {
    static T object{};
    return size_t(&(object.*member)) - size_t(&object);
}

struct TagDataTypeAny;
struct TagDataTypeEnum;
struct TagDataTypeStruct;
struct TagDataTypeUnion;

}  // namespace detail

/**
 * Builder to create DataType definitions of custom types.
 *
 * The attributes `memSize`, `padding`, `pointerFree`, `isArray` and `isOptional` are automatically
 * deduced from the types itself.
 */
template <typename T, typename Tag = detail::TagDataTypeAny, typename U = struct DeferT>
class DataTypeBuilder {
public:
    /**
     * Build a DataType definition for an enum.
     * @param typeName Human-readable type name
     * @param typeId NodeId of the type
     * @param binaryEncodingId NodeId of data type when encoded as binary
     */
    static auto createEnum(std::string_view typeName, NodeId typeId, NodeId binaryEncodingId);

    /**
     * Build a DataType definition for a structure.
     * A structure may have optional fields (pointers).
     * @param typeName Human-readable type name
     * @param typeId NodeId of the type
     * @param binaryEncodingId NodeId of data type when encoded as binary
     */
    static auto createStructure(std::string_view typeName, NodeId typeId, NodeId binaryEncodingId);

    /**
     * Build a DataType definition for an union.
     * Union type consist of a switch field and the actual union.
     * @param typeName Human-readable type name
     * @param typeId NodeId of the type
     * @param binaryEncodingId NodeId of data type when encoded as binary
     */
    static auto createUnion(std::string_view typeName, NodeId typeId, NodeId binaryEncodingId);

    /**
     * Add a structure field.
     * The offset is derived from the member pointer.
     * @tparam field Member pointer, e.g. `&S::value`
     * @param fieldName Human-readable field name
     * @param fieldType Member data type
     */
    template <auto U::*field>
    auto& addField(std::string_view fieldName, const UA_DataType& fieldType) {
        using TMember = detail::MemberTypeT<decltype(field)>;
        return addField<TMember>(fieldName, detail::offsetOfMember(field), fieldType);
    }

    /**
     * Add a structure field (derive DataType from `field`).
     * @overload
     */
    template <auto U::*field>
    auto& addField(std::string_view fieldName) {
        return addField<field>(fieldName, detail::getMemberDataType<field>());
    }

    /**
     * Add a structure field with a manual offset.
     * @tparam TMember Type of the member, e.g. `opcua::String`
     * @param fieldName Human-readable field name
     * @param offset Offset of the member in the structure
     * @param fieldType Member data type
     */
    template <typename TMember>
    auto& addField(std::string_view fieldName, size_t offset, const UA_DataType& fieldType);

    /**
     * Add a structure field with a manual offset (derive DataType from `TMember`).
     * @overload
     */
    template <typename TMember>
    auto& addField(std::string_view fieldName, size_t offset) {
        return addField<TMember>(fieldName, offset, getDataType<std::remove_pointer_t<TMember>>());
    }

    /**
     * Add a structure array field.
     * Arrays must consists of two fields: its size (of type `size_t`) and the pointer to the data.
     * No padding allowed between the size field and the array field.
     * The offset is derived from the member pointer.
     * @tparam fieldSize Member pointer to the size field, e.g. `&S::length`
     * @tparam fieldArray Member pointer to the array field, e.g. `&S::data`
     * @param fieldName Human-readable field name
     * @param fieldType Member data type
     */
    template <size_t U::*fieldSize, auto U::*fieldArray>
    auto& addField(std::string_view fieldName, const UA_DataType& fieldType) {
        using TArray = detail::MemberTypeT<decltype(fieldArray)>;
        return addField<TArray>(
            fieldName,
            detail::offsetOfMember(fieldSize),
            detail::offsetOfMember(fieldArray),
            fieldType
        );
    }

    /**
     * Add a structure array field (derive DataType from `fieldArray`).
     * @overload
     */
    template <size_t U::*fieldSize, auto U::*fieldArray>
    auto& addField(std::string_view fieldName) {
        return addField<fieldSize, fieldArray>(fieldName, detail::getMemberDataType<fieldArray>());
    }

    /**
     * Add a structure array field with a manual offset.
     * @tparam TArray Type of the array field, e.g. `opcua::String`
     * @param fieldName Human-readable field name
     * @param offsetSize Offset of the size field in the structure
     * @param offsetArray Offset of the array field in the structure
     * @param fieldType Array field data type
     */
    template <typename TArray>
    auto& addField(
        std::string_view fieldName,
        size_t offsetSize,
        size_t offsetArray,
        const UA_DataType& fieldType
    );

    /**
     * Add a structure array field with a manual offset (derive DataType from `TArray`).
     * @overload
     */
    template <typename TArray>
    auto& addField(std::string_view fieldName, size_t offsetSize, size_t offsetArray) {
        return addField<TArray>(
            fieldName, offsetSize, offsetArray, getDataType<std::remove_pointer_t<TArray>>()
        );
    }

    /**
     * Add a union field.
     * @tparam memberUnion Member pointer to the union, e.g. `&S::Uni`
     * @tparam TField Type of the union field
     * @param fieldName Human-readable field name
     * @param fieldType Data type of the union field
     */
    template <auto U::*memberUnion, typename TField>
    auto& addUnionField(std::string_view fieldName, const UA_DataType& fieldType);

    /**
     * Add a union field (derive DataType from `TField`).
     * @overload
     */
    template <auto U::*memberUnion, typename TField>
    auto& addUnionField(std::string_view fieldName) {
        return addUnionField<memberUnion, TField>(fieldName, getDataType<TField>());
    }

    /**
     * Create the actual DataType.
     */
    [[nodiscard]] DataType build();

private:
    template <typename, typename, typename>
    friend class DataTypeBuilder;

    explicit DataTypeBuilder(DataType dataType)
        : dataType_{std::move(dataType)} {}

    struct Field {
        size_t memSize{};
        size_t offset{};
        DataTypeMember dataTypeMember;
    };

    DataType dataType_;
    std::vector<Field> fields_;
};

/* --------------------------------------- Implementation --------------------------------------- */

template <typename T, typename Tag, typename U>
auto DataTypeBuilder<T, Tag, U>::createEnum(
    std::string_view typeName, NodeId typeId, NodeId binaryEncodingId
) {
    static_assert(std::is_enum_v<T>, "T must be an enum");
    DataType dt;
    dt.setTypeName(typeName);
    dt.setTypeId(std::move(typeId));
    dt.setBinaryEncodingId(std::move(binaryEncodingId));
    dt.setMemSize(sizeof(T));  // enums must be 32 bit!
    dt.setTypeKind(UA_DATATYPEKIND_ENUM);
    dt.setPointerFree(true);
    dt.setOverlayable(false);
    return DataTypeBuilder<T, detail::TagDataTypeEnum>(dt);
}

template <typename T, typename Tag, typename U>
auto DataTypeBuilder<T, Tag, U>::createStructure(
    std::string_view typeName, NodeId typeId, NodeId binaryEncodingId
) {
    static_assert(std::is_class_v<T>, "T must be a struct or class");
    DataType dt;
    dt.setTypeName(typeName);
    dt.setTypeId(std::move(typeId));
    dt.setBinaryEncodingId(std::move(binaryEncodingId));
    dt.setMemSize(sizeof(T));
    dt.setTypeKind(UA_DATATYPEKIND_STRUCTURE);
    dt.setPointerFree(true);
    dt.setOverlayable(false);
    return DataTypeBuilder<T, detail::TagDataTypeStruct, T>(dt);
}

template <typename T, typename Tag, typename U>
auto DataTypeBuilder<T, Tag, U>::createUnion(
    std::string_view typeName, NodeId typeId, NodeId binaryEncodingId
) {
    static_assert(std::is_class_v<T>, "T must be a struct or class");
    DataType dt;
    dt.setTypeName(typeName);
    dt.setTypeId(std::move(typeId));
    dt.setBinaryEncodingId(std::move(binaryEncodingId));
    dt.setMemSize(sizeof(T));
    dt.setTypeKind(UA_DATATYPEKIND_UNION);
    dt.setPointerFree(true);
    dt.setOverlayable(false);
    return DataTypeBuilder<T, detail::TagDataTypeUnion, T>(dt);
}

template <typename T, typename Tag, typename U>
template <typename TMember>
auto& DataTypeBuilder<T, Tag, U>::addField(
    std::string_view fieldName, size_t offset, const UA_DataType& fieldType
) {
    static_assert(
        std::is_same_v<Tag, detail::TagDataTypeStruct>,
        "Built type must be a struct or class to add members"
    );
    assert(sizeof(std::remove_pointer_t<TMember>) == fieldType.memSize);
    if (std::is_pointer_v<TMember>) {
        dataType_.setTypeKind(UA_DATATYPEKIND_OPTSTRUCT);
    }
    if (std::is_pointer_v<TMember> || !fieldType.pointerFree) {
        dataType_.setPointerFree(false);
    }
    DataTypeMember member;
    member.setMemberName(fieldName);
    member.setMemberType(&fieldType);
    member.setPadding({});  // calculate padding between members later
    member.setIsArray(false);
    member.setIsOptional(std::is_pointer_v<TMember>);
    fields_.push_back({sizeof(TMember), offset, std::move(member)});
    return *this;
}

template <typename T, typename Tag, typename U>
template <typename TArray>
auto& DataTypeBuilder<T, Tag, U>::addField(
    const std::string_view fieldName,
    size_t offsetSize,
    [[maybe_unused]] size_t offsetArray,
    const UA_DataType& fieldType
) {
    static_assert(
        std::is_same_v<Tag, detail::TagDataTypeStruct>,
        "Built type must be a struct or class to add members"
    );
    static_assert(std::is_pointer_v<TArray>, "TArray must be a pointer");
    assert(
        offsetArray == offsetSize + sizeof(size_t) &&
        "No padding between members size and array allowed"
    );
    assert(sizeof(std::remove_pointer_t<TArray>) == fieldType.memSize);
    dataType_.setPointerFree(false);
    DataTypeMember member;
    member.setMemberName(fieldName);
    member.setMemberType(&fieldType);
    member.setPadding({});  // calculate padding between members later
    member.setIsArray(true);
    member.setIsOptional(false);
    fields_.push_back({
        sizeof(size_t) + sizeof(TArray),
        offsetSize,  // offset/padding related to size field
        std::move(member),
    });
    return *this;
}

template <typename T, typename Tag, typename U>
template <auto U::*memberUnion, typename TField>
auto& DataTypeBuilder<T, Tag, U>::addUnionField(
    std::string_view fieldName, const UA_DataType& fieldType
) {
    using TUnion = detail::MemberTypeT<decltype(memberUnion)>;
    static_assert(
        std::is_same_v<Tag, detail::TagDataTypeUnion>,
        "Built type must be a union to add union fields"
    );
    static_assert(std::is_union_v<TUnion>, "TUnion must be a union");
    static_assert(sizeof(TField) <= sizeof(TUnion), "TField exceeds size of union");
    const auto offset = detail::offsetOfMember(memberUnion);
    assert(offset > 0 && "A union type must consist of a switch field and a union");
    assert(sizeof(std::remove_pointer_t<TField>) == fieldType.memSize);
    if (std::is_pointer_v<TField> || !fieldType.pointerFree) {
        dataType_.setPointerFree(false);
    }
    DataTypeMember member;
    member.setMemberName(fieldName);
    member.setMemberType(&fieldType);
    member.setPadding(static_cast<uint8_t>(offset));  // padding = offset of each field
    member.setIsArray(false);
    member.setIsOptional(std::is_pointer_v<TField>);
    fields_.push_back({sizeof(TField), offset, std::move(member)});
    return *this;
}

template <typename T, typename Tag, typename U>
[[nodiscard]] DataType DataTypeBuilder<T, Tag, U>::build() {
    static_assert(!std::is_same_v<Tag, detail::TagDataTypeAny>);
    // sort members by offset
    std::sort(fields_.begin(), fields_.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.offset < rhs.offset;
    });
    // calculate padding of struct members
    if constexpr (std::is_same_v<Tag, detail::TagDataTypeStruct>) {
        for (auto it = fields_.begin(); it < fields_.end(); ++it) {
            if (it == fields_.begin()) {
                it->dataTypeMember->padding = static_cast<uint8_t>(it->offset);
            } else {
                it->dataTypeMember->padding = static_cast<uint8_t>(
                    it->offset - (std::prev(it)->offset + std::prev(it)->memSize)
                );
            }
        }
    }
    // generate and set members array
    std::vector<UA_DataTypeMember> members(fields_.size());
    std::transform(fields_.cbegin(), fields_.cend(), members.begin(), [](const auto& m) {
        return asNative(m.dataTypeMember);  // shallow copy
    });
    dataType_.setMembers({asWrapper<DataTypeMember>(members.data()), members.size()});
    return dataType_;
}

/* --------------------------------------- Free functions --------------------------------------- */

const UA_DataType* findDataType(const NodeId& id) noexcept;
const UA_DataType* findDataType(const NodeId& id, const UA_DataTypeArray* custom) noexcept;

/* ------------------------------------------- Helper ------------------------------------------- */

namespace detail {

void clear(UA_DataTypeMember& native) noexcept;
void clear(UA_DataType& native) noexcept;
void clear(UA_DataTypeArray& native) noexcept;

void deallocate(const UA_DataTypeArray* head) noexcept;

[[nodiscard]] UA_DataTypeMember copy(const UA_DataTypeMember& src);
[[nodiscard]] UA_DataType copy(const UA_DataType& src);

void addDataTypes(const UA_DataTypeArray*& head, Span<const DataType> types);

}  // namespace detail

}  // namespace opcua
