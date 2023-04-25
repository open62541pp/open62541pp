#include "open62541pp/Node.h"

#include <algorithm>  // remove_if

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/services/View.h"
#include "open62541pp/types/Composed.h"

namespace opcua {

template <typename T>
std::vector<ReferenceDescription> Node<T>::getReferences(
    BrowseDirection browseDirection,
    ReferenceType referenceType,
    bool includeSubtypes,
    uint32_t nodeClassMask
) {
    const BrowseDescription bd(
        nodeId_,
        browseDirection,
        referenceType,
        includeSubtypes,
        nodeClassMask,
        UA_BROWSERESULTMASK_ALL
    );
    return services::browseAll(connection_, bd);
}

template <typename T>
std::vector<Node<T>> Node<T>::getReferencedNodes(
    BrowseDirection browseDirection,
    ReferenceType referenceType,
    bool includeSubtypes,
    uint32_t nodeClassMask
) {
    const BrowseDescription bd(
        nodeId_,
        browseDirection,
        referenceType,
        includeSubtypes,
        nodeClassMask,
        UA_BROWSERESULTMASK_TARGETINFO  // only node id required here
    );
    const auto refs = services::browseAll(connection_, bd);
    std::vector<Node<T>> nodes;
    nodes.reserve(refs.size());
    for (const auto& ref : refs) {
        if (ref.getNodeId().isLocal()) {
            nodes.emplace_back(connection_, ref.getNodeId().getNodeId(), false);
        }
    }
    return nodes;
}

template <typename T>
Node<T> Node<T>::getChild(const std::vector<QualifiedName>& path) {
    const auto result = services::browseSimplifiedBrowsePath(connection_, nodeId_, path);
    for (auto&& target : result.getTargets()) {
        if (target.getTargetId().isLocal()) {
            return {connection_, target.getTargetId().getNodeId(), false};
        }
    }
    throw BadStatus(UA_STATUSCODE_BADNOMATCH);
}

template <typename T>
Node<T> Node<T>::getParent() {
    const auto nodes = getReferencedNodes(
        BrowseDirection::Inverse,
        ReferenceType::HierarchicalReferences,
        true,
        UA_NODECLASS_UNSPECIFIED
    );
    if (nodes.empty()) {
        throw BadStatus(UA_STATUSCODE_BADNOTFOUND);
    }
    return nodes[0];
}

/* ---------------------------------------------------------------------------------------------- */

// explicit template instantiation
template class Node<Server>;
template class Node<Client>;

}  // namespace opcua
