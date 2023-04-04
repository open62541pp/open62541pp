//
// Created by ELopes1 on 4/4/2023.
//

#pragma once

#include "open62541pp/NodeId.h"
#include "open62541pp/types.h"

#include "open62541pp/TypeWrapper.h"
#include "open62541pp/Types.h"
#include "open62541pp/open62541.h"

namespace opcua {

class ReferenceDescription
    : public TypeWrapper<UA_ReferenceDescription, UA_TYPES_REFERENCEDESCRIPTION> {
public:
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    explicit ReferenceDescription(
        NodeId referenceTypeId,
        bool isForward,
        ExpandedNodeId nodeId,
        QualifiedName browseName,
        LocalizedText displayName,
        NodeClass nodeClass,
        ExpandedNodeId typeDefinition);

private:
    NodeId referenceTypeId_;
    bool isForward_;
    ExpandedNodeId nodeId_;
    QualifiedName browseName_;
    LocalizedText displayName_;
    NodeClass nodeClass_;
    ExpandedNodeId typeDefinition_;
};

}  // namespace opcua