#pragma once

#include <iterator>  // make_move_iterator
#include <vector>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/TypeWrapper.h"  // isTypeWrapper

namespace opcua::detail {

template <typename Response>
inline auto& getSingleResult(Response& response) {
    if constexpr (isTypeWrapper<Response>) {
        auto results = response.getResults();
        if (results.data() == nullptr || results.size() != 1) {
            throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
        }
        return results[0];
    } else {
        if (response.results == nullptr || response.resultsSize != 1) {
            throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
        }
        return *response.results;
    }
}

inline auto getOutputArguments(UA_CallMethodResult& result) {
    detail::throwOnBadStatus(result.statusCode);
    detail::throwOnBadStatus(result.inputArgumentResults, result.inputArgumentResultsSize);
    return std::vector<Variant>(
        std::make_move_iterator(result.outputArguments),
        std::make_move_iterator(result.outputArguments + result.outputArgumentsSize)  // NOLINT
    );
}

}  // namespace opcua::detail
