#pragma once

#include <type_traits>
#include <utility>  // exchange

#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/detail/result_utils.hpp"
#include "open62541pp/result.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/types.hpp"  // StatusCode
#include "open62541pp/wrapper.hpp"  // asWrapper, isWrapper

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

template <typename Response>
[[nodiscard]] auto moveSingleResultWithStatus(Response& response) noexcept {
    using ResultType = std::remove_pointer_t<decltype(response.results)>;
    if (const StatusCode serviceResult = getServiceResult(response); serviceResult.isBad()) {
        ResultType result{};
        result.statusCode = serviceResult;
        return result;
    }
    if (response.results == nullptr || response.resultsSize != 1) {
        ResultType result{};
        result.statusCode = UA_STATUSCODE_BADUNEXPECTEDERROR;
        return result;
    }
    return std::exchange(*response.results, {});
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

inline Result<uint32_t> getMonitoredItemId(const UA_MonitoredItemCreateResult& result) noexcept {
    if (const StatusCode code = result.statusCode; code.isBad()) {
        return BadResult(code);
    }
    return result.monitoredItemId;
}

}  // namespace opcua::services::detail
