#pragma once

#include <iterator>  // make_move_iterator
#include <vector>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Span.h"
#include "open62541pp/TypeWrapper.h"  // isTypeWrapper
#include "open62541pp/open62541.h"
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

inline std::vector<Variant> getOutputArguments(UA_CallMethodResult& result) {
    throwOnBadStatus(result.statusCode);
    throwOnBadStatus(result.inputArgumentResults, result.inputArgumentResultsSize);
    return {
        std::make_move_iterator(result.outputArguments),
        std::make_move_iterator(result.outputArguments + result.outputArgumentsSize)  // NOLINT
    };
}

}  // namespace opcua::detail
