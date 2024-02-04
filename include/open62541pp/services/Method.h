#pragma once

#include <utility>  // forward
#include <vector>

#include "open62541pp/Client.h"
#include "open62541pp/Config.h"
#include "open62541pp/Span.h"
#include "open62541pp/async.h"
#include "open62541pp/services/detail/ClientService.h"
#include "open62541pp/services/detail/RequestHandling.h"
#include "open62541pp/services/detail/ResponseHandling.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

#ifdef UA_ENABLE_METHODCALLS

namespace opcua::services {

/**
 * @defgroup Method Method service set
 * Call (invoke) methods.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.11
 * @ingroup Services
 * @{
 *
 * @defgroup Call
 * This Service is used to call (invoke) a list of methods.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.11.2
 * @{
 */

/**
 * Call a server method and return results.
 * The `objectId` must have a `HasComponent` reference to the method specified in `methodId`.
 *
 * @param serverOrClient Instance of type Server or Client
 * @param objectId NodeId of the object on which the method is invoked
 * @param methodId NodeId of the method to invoke
 * @param inputArguments Input argument values
 * @exception BadStatus
 * @exception BadStatus (BadInvalidArgument) If input arguments don't match expected variant types
 * @exception BadStatus (BadArgumentsMissing) If input arguments are missing
 * @exception BadStatus (BadTooManyArguments) If too many input arguments provided
 */
template <typename T>
std::vector<Variant> call(
    T& serverOrClient,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
);

/**
 * Asynchronously call a server method and return results.
 *
 * @param client Instance of type Client
 * @param objectId NodeId of the object on which the method is invoked
 * @param methodId NodeId of the method to invoke
 * @param inputArguments Input argument values
 * @param token @completiontoken{void(opcua::StatusCode, std::vector<opcua::Variant>&)}
 * @exception BadStatus
 */
template <typename CompletionToken = DefaultCompletionToken>
auto callAsync(
    Client& client,
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
        client,
        request,
        [](UA_CallResponse& response) {
            return detail::getOutputArguments(detail::getSingleResult(response));
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
