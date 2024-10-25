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

// Get single result from response and save response status code in result type's statusCode member.
template <typename WrapperType, typename Response>
[[nodiscard]] auto wrapSingleResultWithStatus(Response& response) noexcept {
    return getSingleResult(response)
        .transform(Wrap<WrapperType>{})
        .orElse([](UA_StatusCode& code) {
            Result<WrapperType> result;
            result->handle()->statusCode = code;
            return result;
        })
        .value();
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

}  // namespace opcua::services::detail
