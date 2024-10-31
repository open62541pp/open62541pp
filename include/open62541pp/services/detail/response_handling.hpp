#pragma once

#include <functional>  // ref
#include <type_traits>
#include <utility>  // exchange

#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/detail/result_utils.hpp"
#include "open62541pp/result.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/types.hpp"  // StatusCode
#include "open62541pp/wrapper.hpp"  // asNative, asWrapper, isWrapper

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
const UA_ResponseHeader& getResponseHeader(const Response& response) noexcept {
    if constexpr (opcua::detail::isWrapper<Response>) {
        return asNative(response).responseHeader;
    } else {
        return response.responseHeader;
    }
}

template <typename Response>
StatusCode getServiceResult(const Response& response) noexcept {
    return getResponseHeader(response).serviceResult;
}

template <typename Response>
auto getSingleResultRef(Response& response) noexcept {
    auto* native = [&] {
        if constexpr (opcua::detail::isWrapper<Response>) {
            return asNative(&response);
        } else {
            return &response;
        }
    }();
    using ResultType = Result<decltype(std::ref(*native->results))>;
    if (const StatusCode serviceResult = getServiceResult(response); serviceResult.isBad()) {
        return ResultType(BadResult(serviceResult));
    }
    if (native->results == nullptr || native->resultsSize != 1) {
        return ResultType(BadResult(UA_STATUSCODE_BADUNEXPECTEDERROR));
    }
    return ResultType(std::ref(*native->results));
}

template <typename Response>
StatusCode getSingleStatus(const Response& response) noexcept {
    auto result = getSingleResultRef(response);
    return result ? asWrapper<StatusCode>(result->get()) : result.code();
}

template <typename WrapperType, typename Response>
Result<WrapperType> wrapSingleResult(Response& response) noexcept {
    return getSingleResultRef(response).transform(Wrap<WrapperType>{});
}

// Get single result from response and save response status code in result type's statusCode member.
template <typename WrapperType, typename Response>
WrapperType wrapSingleResultWithStatus(Response& response) noexcept {
    return *wrapSingleResult<WrapperType>(response).orElse([](StatusCode code) {
        Result<WrapperType> result;
        result->handle()->statusCode = code;
        return result;
    });
}

inline Result<NodeId> getAddedNodeId(UA_AddNodesResult& result) noexcept {
    if (const StatusCode code = result.statusCode; code.isBad()) {
        return BadResult(code);
    }
    return Wrap<NodeId>{}(result.addedNodeId);
}

}  // namespace opcua::services::detail
