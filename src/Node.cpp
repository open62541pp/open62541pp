#include "open62541pp/Node.h"

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/services/View.h"
#include "open62541pp/types/Composed.h"

#include "open62541_impl.h"

namespace opcua {

template <>
bool Node<Client>::exists() noexcept {
    // create minimal request
    UA_ReadValueId item{};
    item.nodeId = *getNodeId().handle();
    item.attributeId = UA_ATTRIBUTEID_NODECLASS;
    UA_ReadRequest request{};
    request.timestampsToReturn = UA_TIMESTAMPSTORETURN_NEITHER;
    request.nodesToReadSize = 1;
    request.nodesToRead = &item;

    const ReadResponse response = UA_Client_Service_read(getConnection().handle(), request);
    if (response->responseHeader.serviceResult != UA_STATUSCODE_GOOD ||
        response->resultsSize != 1) {
        return false;
    }
    if (response->results->hasStatus && response->results->status != UA_STATUSCODE_GOOD) {
        return false;
    }
    return true;
}

template <>
bool Node<Server>::exists() noexcept {
    void* context{};
    const auto status = UA_Server_getNodeContext(getConnection().handle(), getNodeId(), &context);
    return (status == UA_STATUSCODE_GOOD);
}

template <typename T>
std::vector<ReferenceDescription> Node<T>::browseReferences(
    BrowseDirection browseDirection,
    const NodeId& referenceType,
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
std::vector<Node<T>> Node<T>::browseReferencedNodes(
    BrowseDirection browseDirection,
    const NodeId& referenceType,
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
            nodes.emplace_back(connection_, ref.getNodeId().getNodeId());
        }
    }
    return nodes;
}

template <typename T>
Node<T> Node<T>::browseChild(Span<const QualifiedName> path) {
    const auto result = services::browseSimplifiedBrowsePath(connection_, nodeId_, path);
    for (auto&& target : result.getTargets()) {
        if (target.getTargetId().isLocal()) {
            return {connection_, target.getTargetId().getNodeId()};
        }
    }
    throw BadStatus(UA_STATUSCODE_BADNOMATCH);
}

template <typename T>
Node<T> Node<T>::browseParent() {
    const auto nodes = browseReferencedNodes(
        BrowseDirection::Inverse,
        ReferenceTypeId::HierarchicalReferences,
        true,
        UA_NODECLASS_UNSPECIFIED
    );
    if (nodes.empty()) {
        throw BadStatus(UA_STATUSCODE_BADNOTFOUND);
    }
    return nodes[0];
}

template <typename T>
static Node<T> browseObjectProperty(Node<T>& objectNode, const QualifiedName& propertyName) {
    const BrowsePath bp(
        objectNode.getNodeId(), {{ReferenceTypeId::HasProperty, false, true, propertyName}}
    );
    const auto result = services::translateBrowsePathToNodeIds(objectNode.getConnection(), bp);
    result.getStatusCode().throwIfBad();
    for (auto&& target : result.getTargets()) {
        const auto id = target.getTargetId();
        if (id.isLocal()) {
            return {objectNode.getConnection(), id.getNodeId()};
        }
    }
    throw BadStatus(UA_STATUSCODE_BADNOTFOUND);
}

template <typename T>
Variant Node<T>::readObjectProperty(const QualifiedName& propertyName) {
    return browseObjectProperty(*this, propertyName).readValue();
}

template <typename T>
Node<T>& Node<T>::writeObjectProperty(const QualifiedName& propertyName, const Variant& value) {
    browseObjectProperty(*this, propertyName).writeValue(value);
    return *this;
}

/* ---------------------------------------------------------------------------------------------- */

// explicit template instantiation
template class Node<Server>;
template class Node<Client>;

}  // namespace opcua
