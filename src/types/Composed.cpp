#include "open62541pp/types/Composed.h"

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

}  // namespace opcua
