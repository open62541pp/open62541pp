#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>  // exchange
#include <vector>

#include "open62541pp/Common.h"  // AttributeId
#include "open62541pp/open62541.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

namespace opcua::services::detail {

/**
 * Attribute handler to convert DataValue objects to/from the attribute specific types.
 * Template specializations must be provided for all AttributeIds.
 */
template <AttributeId Attribute>
struct AttributeHandler;

template <typename T, typename Enable = void>
struct AttributeHandlerScalar {
    using Type = T;

    static auto fromDataValue(DataValue&& dv) {
        return dv.getValue().getScalar<Type>();
    }

    template <typename U>
    static auto toDataValue(U&& value) {
        return DataValue::fromScalar(std::forward<U>(value));
    }
};

template <typename T>
struct AttributeHandlerScalar<T, std::enable_if_t<std::is_enum_v<T>>> {
    using Type = T;
    using UnderlyingType = std::underlying_type_t<Type>;

    static auto fromDataValue(DataValue&& dv) {
        return static_cast<Type>(dv.getValue().getScalar<UnderlyingType>());
    }

    static auto toDataValue(Type value) {
        return DataValue::fromScalar(static_cast<UnderlyingType>(value));
    }
};

template <>
struct AttributeHandler<AttributeId::NodeId> : AttributeHandlerScalar<NodeId> {};

template <>
struct AttributeHandler<AttributeId::NodeClass> {
    using Type = NodeClass;

    static auto fromDataValue(DataValue&& dv) noexcept {
        // workaround to read enum from variant...
        return *static_cast<NodeClass*>(dv.getValue().data());
    }
};

template <>
struct AttributeHandler<AttributeId::BrowseName> : AttributeHandlerScalar<QualifiedName> {};

template <>
struct AttributeHandler<AttributeId::DisplayName> : AttributeHandlerScalar<LocalizedText> {};

template <>
struct AttributeHandler<AttributeId::Description> : AttributeHandlerScalar<LocalizedText> {};

template <>
struct AttributeHandler<AttributeId::WriteMask> : AttributeHandlerScalar<uint32_t> {};

template <>
struct AttributeHandler<AttributeId::UserWriteMask> : AttributeHandlerScalar<uint32_t> {};

template <>
struct AttributeHandler<AttributeId::IsAbstract> : AttributeHandlerScalar<bool> {};

template <>
struct AttributeHandler<AttributeId::Symmetric> : AttributeHandlerScalar<bool> {};

template <>
struct AttributeHandler<AttributeId::InverseName> : AttributeHandlerScalar<LocalizedText> {};

template <>
struct AttributeHandler<AttributeId::ContainsNoLoops> : AttributeHandlerScalar<bool> {};

template <>
struct AttributeHandler<AttributeId::EventNotifier> : AttributeHandlerScalar<uint8_t> {};

template <>
struct AttributeHandler<AttributeId::Value> {
    using Type = Variant;

    static Variant fromDataValue(DataValue&& dv) {
        return std::exchange(dv->value, {});
    }

    static auto toDataValue(const Variant& value) {
        DataValue dv;
        dv->value = value;  // shallow copy
        dv->value.storageType = UA_VARIANT_DATA_NODELETE;  // prevent double delete
        dv->hasValue = true;
        return dv;
    }
};

template <>
struct AttributeHandler<AttributeId::DataType> : AttributeHandlerScalar<NodeId> {};

template <>
struct AttributeHandler<AttributeId::ValueRank> : AttributeHandlerScalar<ValueRank> {};

template <>
struct AttributeHandler<AttributeId::ArrayDimensions> {
    using Type = std::vector<uint32_t>;

    static auto fromDataValue(DataValue&& dv) {
        if (dv.getValue().isArray()) {
            return dv.getValue().getArrayCopy<uint32_t>();
        }
        return std::vector<uint32_t>{};
    }

    static auto toDataValue(Span<const uint32_t> dimensions) {
        return DataValue::fromArray(dimensions);
    }
};

template <>
struct AttributeHandler<AttributeId::AccessLevel> : AttributeHandlerScalar<uint8_t> {};

template <>
struct AttributeHandler<AttributeId::UserAccessLevel> : AttributeHandlerScalar<uint8_t> {};

template <>
struct AttributeHandler<AttributeId::MinimumSamplingInterval> : AttributeHandlerScalar<double> {};

template <>
struct AttributeHandler<AttributeId::Historizing> : AttributeHandlerScalar<bool> {};

template <>
struct AttributeHandler<AttributeId::Executable> : AttributeHandlerScalar<bool> {};

}  // namespace opcua::services::detail
