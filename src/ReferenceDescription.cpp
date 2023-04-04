#include "open62541pp/ReferenceDescription.h"

namespace opcua {
ReferenceDescription::ReferenceDescription(
    NodeId referenceTypeId,
    bool isForward,
    ExpandedNodeId nodeId,
    QualifiedName browseName,
    LocalizedText displayName,
    NodeClass nodeClass,
    ExpandedNodeId typeDefinition
) {
    referenceTypeId_ = referenceTypeId;
    isForward_ = isForward;
    nodeId_ = nodeId;
    browseName_ = browseName;
    displayName_ = displayName;
    nodeClass_ = nodeClass;
    typeDefinition_ = typeDefinition;
}
}  // namespace opcua