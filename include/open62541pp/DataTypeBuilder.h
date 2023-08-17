#pragma once

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <utility>  // move
#include <vector>

#include "open62541pp/DataType.h"
#include "open62541pp/TypeConverter.h"
#include "open62541pp/detail/helper.h"
#include "open62541pp/detail/traits.h"
#include "open62541pp/types/NodeId.h"

namespace opcua {

namespace detail {

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
     * Add a structure field.
     * @tparam field Member pointer, e.g. `&S::value`
     * @param fieldName Human-readable field name
     * @param fieldType Member data type
     */
    template <auto T::*field>
    DataTypeBuilder<T, Tag>& addField(const char* fieldName, const UA_DataType& fieldType);

    /**
     * Add a structure field (derive DataType from `field`).
     * @overload
     */
    template <auto T::*field>
    DataTypeBuilder<T, Tag>& addField(const char* fieldName) {
        return addField<field>(fieldName, detail::guessMemberDataType<field>());
    }

    /**
     * Add a structure array field.
     * Arrays must consists of two fields: its size (of type `size_t`) and the pointer to the data.
     * No padding allowed between the size field and the array field.
     * @tparam fieldSize Member pointer to the size field, e.g. `&S::lenth`
     * @tparam fieldArray Member pointer to the array field, e.g. `&S::data`
     * @param fieldName Human-readable field name
     * @param fieldType Member data type
     */
    template <auto T::*fieldSize, auto T::*fieldArray>
    DataTypeBuilder<T, Tag>& addField(const char* fieldName, const UA_DataType& fieldType);

    /**
     * Add a structure array field (derive DataType from `fieldArray`).
     * @overload
     */
    template <auto T::*fieldSize, auto T::*fieldArray>
    DataTypeBuilder<T, Tag>& addField(const char* fieldName) {
        return addField<fieldSize, fieldArray>(
            fieldName, detail::guessMemberDataType<fieldArray>()
        );
    }

    /**
     * Add a union field.
     * @tparam memberUnion Member pointer to the union, e.g. `&S::Uni`
     * @tparam TField Type of the union field
     * @param fieldName Human-readable field name
     * @param fieldType Data type of the union field
     */
    template <auto T::*memberUnion, typename TField>
    DataTypeBuilder<T, Tag>& addUnionField(const char* fieldName, const UA_DataType& fieldType);

    /**
     * Add a union field (derive DataType from `TField`).
     * @overload
     */
    template <auto T::*memberUnion, typename TField>
    DataTypeBuilder<T, Tag>& addUnionField(const char* fieldName) {
        return addUnionField<memberUnion, TField>(fieldName, detail::guessDataType<TField>());
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

    struct Field {
        size_t memSize;
        size_t offset;
        DataTypeMember dataTypeMember;
    };

    DataType dataType_;
    std::vector<Field> fields_;
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
template <auto T::*field>
DataTypeBuilder<T, Tag>& DataTypeBuilder<T, Tag>::addField(
    const char* fieldName, const UA_DataType& fieldType
) {
    using TMember = detail::MemberTypeT<decltype(field)>;
    static_assert(
        std::is_same_v<Tag, detail::TagDataTypeStruct>,
        "Built type must be a struct or class to add members"
    );
    assert(sizeof(detail::UnqualifiedT<TMember>) == fieldType.memSize);
    if (std::is_pointer_v<TMember>) {
        dataType_.setTypeKind(UA_DATATYPEKIND_OPTSTRUCT);
    }
    if (std::is_pointer_v<TMember> || !fieldType.pointerFree) {
        dataType_.setPointerFree(false);
    }
    fields_.push_back({
        sizeof(TMember),
        detail::offsetOfMember(field),
        detail::createDataTypeMember(
            fieldName,
            fieldType,
            {},  // calculate padding between members later
            false,
            std::is_pointer_v<TMember>
        ),
    });
    return *this;
}

template <typename T, typename Tag>
template <auto T::*fieldSize, auto T::*fieldArray>
DataTypeBuilder<T, Tag>& DataTypeBuilder<T, Tag>::addField(
    const char* fieldName, const UA_DataType& fieldType
) {
    using TSize = detail::MemberTypeT<decltype(fieldSize)>;
    using TArray = detail::MemberTypeT<decltype(fieldArray)>;
    static_assert(
        std::is_same_v<Tag, detail::TagDataTypeStruct>,
        "Built type must be a struct or class to add members"
    );
    static_assert(std::is_integral_v<TSize>, "TSize must be an integral type");
    static_assert(std::is_pointer_v<TArray>, "TArray must be a pointer");
    // NOLINTNEXTLINE
    assert(
        detail::offsetOfMember(fieldArray) == detail::offsetOfMember(fieldSize) + sizeof(TSize) &&
        "No padding between members size and array allowed"
    );
    assert(sizeof(detail::UnqualifiedT<TArray>) == fieldType.memSize);
    dataType_.setPointerFree(false);
    fields_.push_back({
        sizeof(TSize) + sizeof(TArray),
        detail::offsetOfMember(fieldSize),  // offset/padding related to size field
        detail::createDataTypeMember(
            fieldName,
            fieldType,
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
    const char* fieldName, const UA_DataType& fieldType
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
    assert(sizeof(detail::UnqualifiedT<TField>) == fieldType.memSize);
    if (std::is_pointer_v<TField> || !fieldType.pointerFree) {
        dataType_.setPointerFree(false);
    }
    fields_.push_back({
        sizeof(TField),
        offset,
        detail::createDataTypeMember(
            fieldName,
            fieldType,
            static_cast<uint8_t>(offset),  // padding = offset of each field
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
    std::sort(fields_.begin(), fields_.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.offset < rhs.offset;
    });
    // calculate padding of struct members
    if constexpr (std::is_same_v<Tag, detail::TagDataTypeStruct>) {
        for (auto it = fields_.begin(); it < fields_.end(); ++it) {
            if (it == fields_.begin()) {
                it->dataTypeMember.padding = static_cast<uint8_t>(it->offset);
            } else {
                it->dataTypeMember.padding = static_cast<uint8_t>(
                    it->offset - (std::prev(it)->offset + std::prev(it)->memSize)
                );
            }
        }
    }
    // generate and set members array
    std::vector<DataTypeMember> dataTypeMembers(fields_.size());
    std::transform(fields_.cbegin(), fields_.cend(), dataTypeMembers.begin(), [](auto&& m) {
        return m.dataTypeMember;
    });
    dataType_.setMembers(dataTypeMembers);
    return dataType_;
}

}  // namespace opcua
