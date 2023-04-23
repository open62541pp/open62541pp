#include "open62541pp/types/Composed.h"

#include "open62541pp/detail/helper.h"

namespace opcua {

BrowseDescription::BrowseDescription(
    const NodeId& nodeId,
    BrowseDirection browseDirection,
    ReferenceType referenceType,
    bool includeSubtypes,
    uint32_t nodeClassMask,  // NOLINT
    uint32_t resultMask  // NOLINT
) {
    asWrapper<NodeId>(handle()->nodeId) = nodeId;
    handle()->browseDirection = static_cast<UA_BrowseDirection>(browseDirection);
    handle()->referenceTypeId = detail::getUaNodeId(referenceType);
    handle()->includeSubtypes = includeSubtypes;
    handle()->nodeClassMask = nodeClassMask;
    handle()->resultMask = resultMask;
}

RelativePathElement::RelativePathElement(
    ReferenceType referenceType,
    bool isInverse,
    bool includeSubtypes,
    const QualifiedName& targetName
) {
    handle()->referenceTypeId = detail::getUaNodeId(referenceType);
    handle()->isInverse = isInverse;
    handle()->includeSubtypes = includeSubtypes;
    asWrapper<QualifiedName>(handle()->targetName) = targetName;
}

RelativePath::RelativePath(std::initializer_list<RelativePathElement> elements) {
    handle()->elementsSize = elements.size();
    handle()->elements = detail::toNativeArrayAlloc(elements.begin(), elements.end());
}

RelativePath::RelativePath(const std::vector<RelativePathElement>& elements) {
    handle()->elementsSize = elements.size();
    handle()->elements = detail::toNativeArrayAlloc(elements.begin(), elements.end());
}

BrowsePath::BrowsePath(const NodeId& startingNode, const RelativePath& relativePath) {
    asWrapper<NodeId>(handle()->startingNode) = startingNode;
    asWrapper<RelativePath>(handle()->relativePath) = relativePath;
}

}  // namespace opcua
