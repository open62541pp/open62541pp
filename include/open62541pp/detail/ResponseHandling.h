#pragma once

#include <iterator>  // make_move_iterator
#include <type_traits>
#include <vector>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Span.h"
#include "open62541pp/TypeWrapper.h"  // isTypeWrapper
#include "open62541pp/open62541.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/Variant.h"

namespace opcua::detail {

template <typename Response>
inline UA_ResponseHeader& getResponseHeader(Response& response) noexcept {
    if constexpr (isTypeWrapper<Response>) {
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
    throwOnBadStatus(getServiceResult(response));
}

template <typename Response>
inline auto getResults(Response& response) {
    checkServiceResult(response);
    if constexpr (isTypeWrapper<Response>) {
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
        throwOnBadStatus(dv.status);
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
 * Attribute tag type for compile-time tag dispatching based on the AttributeId.
 */
template <AttributeId Attribute>
using AttributeTag = std::integral_constant<AttributeId, Attribute>;

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::NodeId> /*unused*/) {
    return dv.getValue().getScalar<NodeId>();
}

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::NodeClass> /*unused*/) {
    // workaround to read enum from variant...
    return *static_cast<NodeClass*>(dv.getValue().data());
}

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::BrowseName> /*unused*/) {
    return dv.getValue().getScalar<QualifiedName>();
}

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::DisplayName> /*unused*/) {
    return dv.getValue().getScalar<LocalizedText>();
}

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::Description> /*unused*/) {
    return dv.getValue().getScalar<LocalizedText>();
}

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::WriteMask> /*unused*/) {
    return dv.getValue().getScalar<uint32_t>();
}

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::UserWriteMask> /*unused*/) {
    return dv.getValue().getScalar<uint32_t>();
}

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::IsAbstract> /*unused*/) {
    return dv.getValue().getScalar<bool>();
}

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::Symmetric> /*unused*/) {
    return dv.getValue().getScalar<bool>();
}

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::InverseName> /*unused*/) {
    return dv.getValue().getScalar<LocalizedText>();
}

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::ContainsNoLoops> /*unused*/) {
    return dv.getValue().getScalar<bool>();
}

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::EventNotifier> /*unused*/) {
    return dv.getValue().getScalar<uint8_t>();
}

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::Value> /*unused*/) {
    return dv.getValue();
}

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::DataType> /*unused*/) {
    return dv.getValue().getScalar<NodeId>();
}

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::ValueRank> /*unused*/) {
    return static_cast<ValueRank>(dv.getValue().getScalar<int32_t>());
}

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::ArrayDimensions> /*unused*/) {
    if (dv.getValue().isArray()) {
        return dv.getValue().getArrayCopy<uint32_t>();
    }
    return std::vector<uint32_t>{};
}

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::AccessLevel> /*unused*/) {
    return dv.getValue().getScalar<uint8_t>();
}

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::UserAccessLevel> /*unused*/) {
    return dv.getValue().getScalar<uint8_t>();
}

inline auto
getAttribute(DataValue&& dv, AttributeTag<AttributeId::MinimumSamplingInterval> /*unused*/) {
    return dv.getValue().getScalar<double>();
}

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::Historizing> /*unused*/) {
    return dv.getValue().getScalar<bool>();
}

inline auto getAttribute(DataValue&& dv, AttributeTag<AttributeId::Executable> /*unused*/) {
    return dv.getValue().getScalar<bool>();
}

inline std::vector<Variant> getOutputArguments(UA_CallMethodResult& result) {
    throwOnBadStatus(result.statusCode);
    throwOnBadStatus(result.inputArgumentResults, result.inputArgumentResultsSize);
    return {
        std::make_move_iterator(result.outputArguments),
        std::make_move_iterator(result.outputArguments + result.outputArgumentsSize)  // NOLINT
    };
}

}  // namespace opcua::detail
