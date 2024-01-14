#pragma once

#include <algorithm>  // for_each_n
#include <iterator>  // make_move_iterator
#include <type_traits>
#include <vector>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Span.h"
#include "open62541pp/TypeWrapper.h"  // isTypeWrapper
#include "open62541pp/open62541.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/Variant.h"

namespace opcua::services::detail {

template <typename Response>
inline UA_ResponseHeader& getResponseHeader(Response& response) noexcept {
    if constexpr (opcua::detail::isTypeWrapper<Response>) {
        return response->responseHeader;
    } else {
        return response.responseHeader;
    }
}

template <typename Response>
inline UA_StatusCode getServiceResult(Response& response) noexcept {
    return getResponseHeader(response).serviceResult;
}

template <typename Response>
inline void checkServiceResult(Response& response) {
    throwIfBad(getServiceResult(response));
}

template <typename Response>
inline auto getResults(Response& response) {
    checkServiceResult(response);
    if constexpr (opcua::detail::isTypeWrapper<Response>) {
        return response.getResults();
    } else {
        return Span(response.results, response.resultsSize);
    }
}

template <typename Response>
inline auto& getSingleResult(Response& response) {
    auto results = getResults(response);
    if (results.data() == nullptr || results.size() != 1) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
    return results[0];
}

inline void checkReadResult(const UA_DataValue& dv) {
    if (dv.hasStatus) {
        throwIfBad(dv.status);
    }
    if (!dv.hasValue) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
}

inline DataValue getReadResult(UA_ReadResponse& response) {
    auto& result = getSingleResult(response);
    checkReadResult(result);
    return {std::exchange(result, {})};
}

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
};

template <typename T>
struct AttributeHandlerScalar<T, std::enable_if_t<std::is_enum_v<T>>> {
    using Type = T;
    using UnderlyingType = std::underlying_type_t<T>;

    static auto fromDataValue(DataValue&& dv) {
        return static_cast<Type>(dv.getValue().getScalar<UnderlyingType>());
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

    static auto fromDataValue(DataValue&& dv) {
        return dv.getValue();
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

inline std::vector<Variant> getOutputArguments(UA_CallMethodResult& result) {
    throwIfBad(result.statusCode);
    std::for_each_n(result.inputArgumentResults, result.inputArgumentResultsSize, throwIfBad);
    return {
        std::make_move_iterator(result.outputArguments),
        std::make_move_iterator(result.outputArguments + result.outputArgumentsSize)  // NOLINT
    };
}

}  // namespace opcua::services::detail
