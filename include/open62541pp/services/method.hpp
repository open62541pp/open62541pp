#pragma once

#include <utility>  // forward
#include <vector>

#include "open62541pp/async.hpp"
#include "open62541pp/config.hpp"
#include "open62541pp/result.hpp"
#include "open62541pp/services/detail/client_services.hpp"
#include "open62541pp/services/detail/request_handling.hpp"
#include "open62541pp/services/detail/response_handling.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/types_composed.hpp"

#ifdef UA_ENABLE_METHODCALLS

namespace opcua {
class Client;
}  // namespace opcua

namespace opcua::services {

/**
 * @defgroup Method Method service set
 * Call (invoke) methods.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.11
 * @ingroup Services
 * @{
 *
 * @defgroup Call Call service
 * This Service is used to call (invoke) a list of methods.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.11.2
 * @{
 */

/**
 * Call server methods.
 * @param connection Instance of type Client
 * @param request Call request
 */
CallResponse call(Client& connection, const CallRequest& request) noexcept;

/**
 * Asynchronously call server methods.
 * @copydetails call
 * @param token @completiontoken{void(CallResponse&)}
 * @return @asyncresult{CallResponse}
 */
template <typename CompletionToken = DefaultCompletionToken>
auto callAsync(
    Client& connection,
    const CallRequest& request,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::sendRequest<UA_CallRequest, UA_CallResponse>(
        connection, request, detail::Wrap<CallResponse>{}, std::forward<CompletionToken>(token)
    );
}

/**
 * Call a server method and return outputs.
 * The `objectId` must have a `HasComponent` reference to the method specified in `methodId`.
 *
 * @param connection Instance of type Server or Client
 * @param objectId NodeId of the object on which the method is invoked
 * @param methodId NodeId of the method to invoke
 * @param inputArguments Input argument values
 * @exception BadStatus
 * @exception BadStatus (BadInvalidArgument) If input arguments don't match expected variant types
 * @exception BadStatus (BadArgumentsMissing) If input arguments are missing
 * @exception BadStatus (BadTooManyArguments) If too many input arguments provided
 */
template <typename T>
Result<std::vector<Variant>> call(
    T& connection,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
) noexcept;

/**
 * Asynchronously call a server method and return outputs.
 *
 * @param connection Instance of type Client
 * @param objectId NodeId of the object on which the method is invoked
 * @param methodId NodeId of the method to invoke
 * @param inputArguments Input argument values
 * @param token @completiontoken{void(Result<std::vector<Variant>>&)}
 * @return @asyncresult{Result<std::vector<Variant>>}
 * @exception BadStatus
 */
template <typename CompletionToken = DefaultCompletionToken>
auto callAsync(
    Client& connection,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments,
    CompletionToken&& token = DefaultCompletionToken()
) {
    UA_CallMethodRequest item = detail::createCallMethodRequest(objectId, methodId, inputArguments);
    UA_CallRequest request{};
    request.methodsToCall = &item;
    request.methodsToCallSize = 1;
    return detail::sendRequest<UA_CallRequest, UA_CallResponse>(
        connection,
        request,
        [](UA_CallResponse& response) {
            return detail::getSingleResult(response).andThen(detail::getOutputArguments);
        },
        std::forward<CompletionToken>(token)
    );
}

/**
 * @}
 * @}
 */

}  // namespace opcua::services

#endif
