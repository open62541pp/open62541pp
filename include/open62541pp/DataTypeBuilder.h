#pragma once

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <utility>  // move

#include "open62541pp/DataType.h"
#include "open62541pp/TypeConverter.h"
#include "open62541pp/detail/helper.h"
#include "open62541pp/detail/traits.h"
#include "open62541pp/types/NodeId.h"

namespace opcua {

namespace detail {

template <typename T>
constexpr const UA_DataType& guessDataType() {
    using ValueType = typename detail::UnqualifiedT<T>;
    static_assert(
        TypeConverter<ValueType>::ValidTypes::size() == 1,
        "Ambiguous member type, please specify data type manually"
    );
    constexpr auto typeIndex = TypeConverter<ValueType>::ValidTypes::toArray().at(0);
    return *detail::getUaDataType(typeIndex);
}

template <auto memberPtr>
constexpr const UA_DataType& guessMemberDataType() {
    return guessDataType<detail::MemberTypeT<decltype(memberPtr)>>();
}

// https://gist.github.com/graphitemaster/494f21190bb2c63c5516
template <typename T, typename TMember>
inline size_t offsetOfMember(TMember T::*member) {
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
template <typename T, typename Tag = detail::TagDataTypeAny>
class DataTypeBuilder {
public:
    /**
     * Build a DataType definition for an enum.
     * @param typeName Human-readable type name
     * @param typeId NodeId of the type
     * @param binaryEncodingId NodeId of data type when encoded as binary
     */
    static auto createEnum(const char* typeName, NodeId typeId, NodeId binaryEncodingId);

    /**
     * Build a DataType definition for a structure.
     * A structure may have optional fields (pointers).
     * @param typeName Human-readable type name
     * @param typeId NodeId of the type
     * @param binaryEncodingId NodeId of data type when encoded as binary
     */
    static auto createStructure(const char* typeName, NodeId typeId, NodeId binaryEncodingId);

    /**
     * Build a DataType definition for an union.
     * Union type consist of a switch field and the actual union.
     * @param typeName Human-readable type name
     * @param typeId NodeId of the type
     * @param binaryEncodingId NodeId of data type when encoded as binary
     */
    static auto createUnion(const char* typeName, NodeId typeId, NodeId binaryEncodingId);

    /**
     * Add a structure member.
     * @tparam member Member pointer, e.g. `&S::value`
     * @param memberName Human-readable member name
     * @param memberType Member data type
     */
    template <auto T::*member>
    DataTypeBuilder<T, Tag>& addMember(const char* memberName, const UA_DataType& memberType);

    /**
     * Add a structure member (derive DataType from `member`).
     * @overload
     */
    template <auto T::*member>
    DataTypeBuilder<T, Tag>& addMember(const char* memberName) {
        return addMember<member>(memberName, detail::guessMemberDataType<member>());
    }

    /**
     * Add a structure array member.
     * Arrays must consists of two fields: its size (of type `size_t`) and the pointer to the data.
     * No padding allowed between the size field and the array field.
     * @tparam memberSize Member pointer to the size field, e.g. `&S::lenth`
     * @tparam memberArray Member pointer to the array field, e.g. `&S::data`
     * @param memberName Human-readable member name
     * @param memberType Member data type
     */
    template <auto T::*memberSize, auto T::*memberArray>
    DataTypeBuilder<T, Tag>& addMember(const char* memberName, const UA_DataType& memberType);

    /**
     * Add a structure array member (derive DataType from `memberArray`).
     * @overload
     */
    template <auto T::*memberSize, auto T::*memberArray>
    DataTypeBuilder<T, Tag>& addMember(const char* memberName) {
        return addMember<memberSize, memberArray>(
            memberName, detail::guessMemberDataType<memberArray>()
        );
    }

    /**
     * Add a union field.
     * @tparam memberUnion Member pointer to the union, e.g. `&S::Uni`
     * @tparam TField Type of the union field
     * @param memberName Human-readable member name
     * @param memberType Data type of the union field
     */
    template <auto T::*memberUnion, typename TField>
    DataTypeBuilder<T, Tag>& addUnionField(const char* memberName, const UA_DataType& memberType);

    /**
     * Add a union field (derive DataType from `TField`).
     * @overload
     */
    template <auto T::*memberUnion, typename TField>
    DataTypeBuilder<T, Tag>& addUnionField(const char* memberName) {
        return addUnionField<memberUnion, TField>(memberName, detail::guessDataType<TField>());
    }

    /**
     * Create the actual DataType.
     */
    [[nodiscard]] DataType build();

private:
    template <typename, typename>
    friend class DataTypeBuilder;

    explicit DataTypeBuilder(DataType dataType)
        : dataType_(std::move(dataType)) {}

    struct Member {
        size_t memSize;
        size_t offset;
        DataTypeMember member;
    };

    DataType dataType_;
    std::vector<Member> members_;
};

/* --------------------------------------- Implementation --------------------------------------- */

template <typename T, typename Tag>
auto DataTypeBuilder<T, Tag>::createEnum(
    const char* typeName, NodeId typeId, NodeId binaryEncodingId
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

template <typename T, typename Tag>
auto DataTypeBuilder<T, Tag>::createStructure(
    const char* typeName, NodeId typeId, NodeId binaryEncodingId
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
    return DataTypeBuilder<T, detail::TagDataTypeStruct>(dt);
}

template <typename T, typename Tag>
auto DataTypeBuilder<T, Tag>::createUnion(
    const char* typeName, NodeId typeId, NodeId binaryEncodingId
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
    return DataTypeBuilder<T, detail::TagDataTypeUnion>(dt);
}

template <typename T, typename Tag>
template <auto T::*member>
DataTypeBuilder<T, Tag>& DataTypeBuilder<T, Tag>::addMember(
    const char* memberName, const UA_DataType& memberType
) {
    using TMember = detail::MemberTypeT<decltype(member)>;
    static_assert(
        std::is_same_v<Tag, detail::TagDataTypeStruct>,
        "Built type must be a struct or class to add members"
    );
    assert(sizeof(detail::UnqualifiedT<TMember>) == memberType.memSize);  // NOLINT
    if (std::is_pointer_v<TMember>) {
        dataType_.setTypeKind(UA_DATATYPEKIND_OPTSTRUCT);
    }
    if (std::is_pointer_v<TMember> || !memberType.pointerFree) {
        dataType_.setPointerFree(false);
    }
    members_.push_back({
        sizeof(TMember),
        detail::offsetOfMember(member),
        detail::createDataTypeMember(
            memberName,
            memberType,
            {},  // calculate padding between members later
            false,
            std::is_pointer_v<TMember>
        ),
    });
    return *this;
}

template <typename T, typename Tag>
template <auto T::*memberSize, auto T::*memberArray>
DataTypeBuilder<T, Tag>& DataTypeBuilder<T, Tag>::addMember(
    const char* memberName, const UA_DataType& memberType
) {
    using TSize = detail::MemberTypeT<decltype(memberSize)>;
    using TArray = detail::MemberTypeT<decltype(memberArray)>;
    static_assert(
        std::is_same_v<Tag, detail::TagDataTypeStruct>,
        "Built type must be a struct or class to add members"
    );
    static_assert(std::is_integral_v<TSize>, "TSize must be an integral type");
    static_assert(std::is_pointer_v<TArray>, "TArray must be a pointer");
    // NOLINTNEXTLINE
    assert(
        detail::offsetOfMember(memberArray) == detail::offsetOfMember(memberSize) + sizeof(TSize) &&
        "No padding between members size and array allowed"
    );
    assert(sizeof(detail::UnqualifiedT<TArray>) == memberType.memSize);  // NOLINT
    dataType_.setPointerFree(false);
    members_.push_back({
        sizeof(TSize) + sizeof(TArray),
        detail::offsetOfMember(memberSize),  // offset/padding related to size member
        detail::createDataTypeMember(
            memberName,
            memberType,
            {},  // calculate padding between members later
            true,
            false
        ),
    });
    return *this;
}

template <typename T, typename Tag>
template <auto T::*memberUnion, typename TField>
DataTypeBuilder<T, Tag>& DataTypeBuilder<T, Tag>::addUnionField(
    const char* memberName, const UA_DataType& memberType
) {
    using TUnion = detail::MemberTypeT<decltype(memberUnion)>;
    static_assert(
        std::is_same_v<Tag, detail::TagDataTypeUnion>,
        "Built type must be a union to add union fields"
    );
    static_assert(std::is_union_v<TUnion>, "TUnion must be a union");
    static_assert(sizeof(TField) <= sizeof(TUnion), "TField exceeds size of union");
    const auto offset = detail::offsetOfMember(memberUnion);
    assert(offset > 0 && "A union type must consist of a switch field and a union");  // NOLINT
    assert(sizeof(detail::UnqualifiedT<TField>) == memberType.memSize);  // NOLINT
    if (std::is_pointer_v<TField> || !memberType.pointerFree) {
        dataType_.setPointerFree(false);
    }
    members_.push_back({
        sizeof(TField),
        offset,
        detail::createDataTypeMember(
            memberName,
            memberType,
            offset,  // padding = offset of each field
            false,
            std::is_pointer_v<TField>
        ),
    });
    return *this;
}

template <typename T, typename Tag>
[[nodiscard]] DataType DataTypeBuilder<T, Tag>::build() {
    static_assert(!std::is_same_v<Tag, detail::TagDataTypeAny>);
    // sort members by offset
    std::sort(members_.begin(), members_.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.offset < rhs.offset;
    });
    // calculate padding of struct members
    if constexpr (std::is_same_v<Tag, detail::TagDataTypeStruct>) {
        for (auto it = members_.begin(); it < members_.end(); ++it) {
            if (it == members_.begin()) {
                it->member.padding = static_cast<uint8_t>(it->offset);
            } else {
                it->member.padding = static_cast<uint8_t>(
                    it->offset - (std::prev(it)->offset + std::prev(it)->memSize)
                );
            }
        }
    }
    // generate and set members array
    std::vector<DataTypeMember> dataTypeMembers(members_.size());
    std::transform(members_.cbegin(), members_.cend(), dataTypeMembers.begin(), [](auto&& m) {
        return m.member;
    });
    dataType_.setMembers(dataTypeMembers);
    return dataType_;
}

}  // namespace opcua
