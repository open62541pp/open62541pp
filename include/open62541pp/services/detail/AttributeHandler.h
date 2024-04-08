#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>  // exchange, move
#include <vector>

#include "open62541pp/Bitmask.h"
#include "open62541pp/Common.h"  // AttributeId, WriteMask, EventNotifier, AccessLevel
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/detail/open62541/common.h"
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

inline Result<Variant> getVariant(DataValue&& dv) {
    if (dv.getStatus().isBad()) {
        return BadResult(dv.getStatus());
    }
    if (!dv.hasValue()) {
        return BadResult(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
    return std::move(dv).getValue();
}

template <typename T, typename Enable = void>
struct AttributeHandlerScalar {
    using Type = T;

    static Result<Type> fromDataValue(DataValue&& dv) {
        return getVariant(std::move(dv)).andThen([](Variant&& var) -> Result<Type> {
            if (var.isScalar() && var.isType<Type>()) {
                return var.getScalar<Type>();
            }
            return BadResult(UA_STATUSCODE_BADUNEXPECTEDERROR);
        });
    }

    template <typename U>
    static DataValue toDataValue(U&& value) {
        return DataValue::fromScalar(std::forward<U>(value));
    }
};

template <typename T>
struct AttributeHandlerScalar<T, std::enable_if_t<std::is_enum_v<T>>> {
    using Type = T;
    using UnderlyingType = std::underlying_type_t<Type>;

    static Result<Type> fromDataValue(DataValue&& dv) {
        return AttributeHandlerScalar<UnderlyingType>::fromDataValue(std::move(dv))
            .transform([](auto value) { return static_cast<Type>(value); });
    }

    static DataValue toDataValue(Type value) {
        return DataValue::fromScalar(static_cast<UnderlyingType>(value));
    }
};

template <typename T>
struct AttributeHandlerScalar<Bitmask<T>> {
    using Type = Bitmask<T>;
    using UnderlyingType = typename Bitmask<T>::Underlying;

    static Result<Type> fromDataValue(DataValue&& dv) {
        return AttributeHandlerScalar<UnderlyingType>::fromDataValue(std::move(dv))
            .transform([](auto value) { return Bitmask<T>(value); });
    }

    static DataValue toDataValue(Type value) {
        return DataValue::fromScalar(value.get());
    }
};

template <>
struct AttributeHandler<AttributeId::NodeId> : AttributeHandlerScalar<NodeId> {};

template <>
struct AttributeHandler<AttributeId::NodeClass> {
    using Type = NodeClass;

    static Result<Type> fromDataValue(DataValue&& dv) noexcept {
        return getVariant(std::move(dv)).transform([](Variant&& var) {
            // workaround to read enum from variant...
            return *static_cast<NodeClass*>(var.data());
        });
    }
};

template <>
struct AttributeHandler<AttributeId::BrowseName> : AttributeHandlerScalar<QualifiedName> {};

template <>
struct AttributeHandler<AttributeId::DisplayName> : AttributeHandlerScalar<LocalizedText> {};

template <>
struct AttributeHandler<AttributeId::Description> : AttributeHandlerScalar<LocalizedText> {};

template <>
struct AttributeHandler<AttributeId::WriteMask> : AttributeHandlerScalar<Bitmask<WriteMask>> {};

template <>
struct AttributeHandler<AttributeId::UserWriteMask> : AttributeHandlerScalar<Bitmask<WriteMask>> {};

template <>
struct AttributeHandler<AttributeId::IsAbstract> : AttributeHandlerScalar<bool> {};

template <>
struct AttributeHandler<AttributeId::Symmetric> : AttributeHandlerScalar<bool> {};

template <>
struct AttributeHandler<AttributeId::InverseName> : AttributeHandlerScalar<LocalizedText> {};

template <>
struct AttributeHandler<AttributeId::ContainsNoLoops> : AttributeHandlerScalar<bool> {};

template <>
struct AttributeHandler<AttributeId::EventNotifier>
    : AttributeHandlerScalar<Bitmask<EventNotifier>> {};

template <>
struct AttributeHandler<AttributeId::Value> {
    using Type = Variant;

    static Result<Variant> fromDataValue(DataValue&& dv) {
        return getVariant(std::move(dv));
    }

    static DataValue toDataValue(const Variant& value) {
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

    static Result<Type> fromDataValue(DataValue&& dv) {
        return getVariant(std::move(dv)).andThen([](Variant&& var) -> Result<Type> {
            if (var.isArray() && var.isType<uint32_t>()) {
                return std::move(var).getArrayCopy<uint32_t>();  // TODO: might throw
            }
            return BadResult(UA_STATUSCODE_BADUNEXPECTEDERROR);
        });
    }

    static DataValue toDataValue(Span<const uint32_t> dimensions) {
        return DataValue::fromArray(dimensions);
    }
};

template <>
struct AttributeHandler<AttributeId::AccessLevel> : AttributeHandlerScalar<Bitmask<AccessLevel>> {};

template <>
struct AttributeHandler<AttributeId::UserAccessLevel>
    : AttributeHandlerScalar<Bitmask<AccessLevel>> {};

template <>
struct AttributeHandler<AttributeId::MinimumSamplingInterval> : AttributeHandlerScalar<double> {};

template <>
struct AttributeHandler<AttributeId::Historizing> : AttributeHandlerScalar<bool> {};

template <>
struct AttributeHandler<AttributeId::Executable> : AttributeHandlerScalar<bool> {};

template <>
struct AttributeHandler<AttributeId::UserExecutable> : AttributeHandlerScalar<bool> {};

}  // namespace opcua::services::detail
