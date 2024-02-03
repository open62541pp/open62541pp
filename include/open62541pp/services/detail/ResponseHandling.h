#pragma once

#include <algorithm>  // for_each_n
#include <iterator>  // make_move_iterator
#include <type_traits>
#include <utility>  // exchange
#include <vector>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Span.h"
#include "open62541pp/TypeWrapper.h"  // isTypeWrapper
#include "open62541pp/open62541.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/Variant.h"

namespace opcua::services::detail {

template <typename WrapperType>
struct WrapResponse {
    static_assert(opcua::detail::isTypeWrapper<WrapperType>);

    template <typename Response = typename WrapperType::NativeType>
    [[nodiscard]] constexpr WrapperType operator()(Response&& value) noexcept {
        return {std::exchange(std::forward<Response>(value), {})};
    }
};

template <typename Response>
inline const UA_ResponseHeader& getResponseHeader(const Response& response) noexcept {
    if constexpr (opcua::detail::isTypeWrapper<Response>) {
        return response->responseHeader;
    } else {
        return response.responseHeader;
    }
}

template <typename Response>
inline UA_StatusCode getServiceResult(const Response& response) noexcept {
    return getResponseHeader(response).serviceResult;
}

template <typename Response>
inline void checkServiceResult(const Response& response) {
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

template <typename Response>
inline auto getSingleResultMove(Response& response) {
    return std::exchange(getSingleResult(response), {});
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

inline std::vector<Variant> getOutputArguments(UA_CallMethodResult& result) {
    throwIfBad(result.statusCode);
    std::for_each_n(result.inputArgumentResults, result.inputArgumentResultsSize, throwIfBad);
    return {
        std::make_move_iterator(result.outputArguments),
        std::make_move_iterator(result.outputArguments + result.outputArgumentsSize)  // NOLINT
    };
}

}  // namespace opcua::services::detail
