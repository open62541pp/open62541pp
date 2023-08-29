#pragma once

#include <vector>

#include "open62541pp/Config.h"
#include "open62541pp/Span.h"

#ifdef UA_ENABLE_METHODCALLS

// forward declarations
namespace opcua {
class NodeId;
class Variant;
}  // namespace opcua

namespace opcua::services {

/**
 * @defgroup Method Method service set
 * Call (invoke) methods.
 *
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/4.7
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.11
 * @ingroup Services
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
 * @ingroup Method
 */
template <typename T>
std::vector<Variant> call(
    T& serverOrClient,
    const NodeId& objectId,
    const NodeId& methodId,
    Span<const Variant> inputArguments
);

}  // namespace opcua::services

#endif
