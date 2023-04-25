#include "open62541pp/Node.h"

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/services/View.h"

namespace opcua {

template <typename T>
Node<T> Node<T>::getChild(const std::vector<QualifiedName>& path) {
    const auto result = services::browseSimplifiedBrowsePath(connection_, nodeId_, path);
    if (result->targetsSize < 1) {
        throw BadStatus(UA_STATUSCODE_BADNOMATCH);
    }
    const auto id = ExpandedNodeId(result->targets[0].targetId);  // NOLINT
    if (!id.isLocal()) {
        throw BadStatus(UA_STATUSCODE_BADNOMATCH);
    }
    return {connection_, id.getNodeId(), false};
}

/* ---------------------------------------------------------------------------------------------- */

// explicit template instantiation
template class Node<Server>;
template class Node<Client>;

}  // namespace opcua
