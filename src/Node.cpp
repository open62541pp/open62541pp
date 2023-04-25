#include "open62541pp/Node.h"

#include <algorithm>  // remove_if

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/services/View.h"
#include "open62541pp/types/Composed.h"

namespace opcua {

inline static void removeNonLocal(std::vector<ReferenceDescription>& refs) {
    refs.erase(
        std::remove_if(
            refs.begin(), refs.end(), [](const auto& ref) { return !ref.getNodeId().isLocal(); }
        ),
        refs.end()
    );
}

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

template <typename T>
Node<T> Node<T>::getParent() {
    const BrowseDescription bd(
        nodeId_,
        BrowseDirection::Inverse,
        ReferenceType::HierarchicalReferences,
        true,  // include subtypes
        UA_NODECLASS_UNSPECIFIED,
        UA_BROWSERESULTMASK_ALL
    );
    auto refs = services::browseAll(connection_, bd);
    removeNonLocal(refs);
    if (refs.empty()) {
        throw BadStatus(UA_STATUSCODE_BADNOTFOUND);
    }
    return {connection_, refs[0].getNodeId().getNodeId(), false};
}

/* ---------------------------------------------------------------------------------------------- */

// explicit template instantiation
template class Node<Server>;
template class Node<Client>;

}  // namespace opcua
