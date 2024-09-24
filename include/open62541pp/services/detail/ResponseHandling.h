#pragma once

#include <algorithm>  // for_each_n
#include <iterator>  // make_move_iterator
#include <type_traits>
#include <utility>  // exchange
#include <vector>

#include "open62541pp/Result.h"
#include "open62541pp/Span.h"
#include "open62541pp/Wrapper.h"  // asWrapper, isWrapper
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/detail/result_util.h"
#include "open62541pp/types/Builtin.h"  // StatusCode
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/Variant.h"

namespace opcua::services::detail {

template <typename WrapperType>
struct Wrap {
    static_assert(opcua::detail::isWrapper<WrapperType>);
    using NativeType = typename WrapperType::NativeType;

    [[nodiscard]] constexpr WrapperType operator()(const NativeType& native) noexcept {
        return {native};
    }

    [[nodiscard]] constexpr WrapperType operator()(NativeType& native) noexcept {
        return {std::exchange(native, {})};
    }

    [[nodiscard]] constexpr WrapperType operator()(NativeType&& native) noexcept {
        return {std::exchange(std::move(native), {})};
    }
};

template <typename Response>
inline const UA_ResponseHeader& getResponseHeader(const Response& response) noexcept {
    if constexpr (opcua::detail::isWrapper<Response>) {
        return response->responseHeader;
    } else {
        return response.responseHeader;
    }
}

template <typename Response>
inline StatusCode getServiceResult(const Response& response) noexcept {
    return getResponseHeader(response).serviceResult;
}

template <typename Response>
auto getSingleResult(Response& response) noexcept -> Result<decltype(std::ref(*response.results))> {
    if (const StatusCode serviceResult = getServiceResult(response); serviceResult.isBad()) {
        return BadResult(serviceResult);
    }
    if (response.results == nullptr || response.resultsSize != 1) {
        return BadResult(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
    return std::ref(*response.results);
}

inline Result<void> toResult(UA_StatusCode code) noexcept {
    if (opcua::detail::isBad(code)) {
        return BadResult(code);
    }
    return {code};
}

inline Result<NodeId> getAddedNodeId(UA_AddNodesResult& result) noexcept {
    if (const StatusCode code = result.statusCode; code.isBad()) {
        return BadResult(code);
    }
    return {std::exchange(result.addedNodeId, {})};
}

inline Result<std::vector<Variant>> getOutputArguments(UA_CallMethodResult& result) noexcept {
    if (const StatusCode code = result.statusCode; code.isBad()) {
        return BadResult(result.statusCode);
    }
    for (const StatusCode code : Span(result.inputArgumentResults, result.inputArgumentResultsSize)) {
        if (code.isBad()) {
            return BadResult(code);
        }
    }
    return opcua::detail::tryInvoke([&] {
        return std::vector<Variant>{
            std::make_move_iterator(result.outputArguments),
            std::make_move_iterator(result.outputArguments + result.outputArgumentsSize)  // NOLINT
        };
    });
}

template <typename SubscriptionParameters, typename Response>
inline void reviseSubscriptionParameters(
    SubscriptionParameters& parameters, const Response& response
) noexcept {
    parameters.publishingInterval = response.revisedPublishingInterval;
    parameters.lifetimeCount = response.revisedLifetimeCount;
    parameters.maxKeepAliveCount = response.revisedMaxKeepAliveCount;
}

template <typename MonitoringParameters, typename Result>
inline void reviseMonitoringParameters(
    MonitoringParameters& parameters, const Result& result
) noexcept {
    // result type may be UA_MonitoredItemCreateResult or UA_MonitoredItemModifyResult
    parameters.samplingInterval = result.revisedSamplingInterval;
    parameters.queueSize = result.revisedQueueSize;
    parameters.filter = asWrapper<ExtensionObject>(result.filterResult);
}

inline Result<uint32_t> getMonitoredItemId(const UA_MonitoredItemCreateResult& result) noexcept {
    if (const StatusCode code = result.statusCode; code.isBad()) {
        return BadResult(code);
    }
    return result.monitoredItemId;
}

}  // namespace opcua::services::detail
