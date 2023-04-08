//
// Created by ELopes1 on 4/4/2023.
//

#pragma once

#include "open62541pp/NodeId.h"
#include "open62541pp/Types.h"

#include "open62541pp/TypeWrapper.h"
#include "open62541pp/Types.h"
#include "open62541pp/open62541.h"

namespace opcua {

class ReferenceDescription
    : public TypeWrapper<UA_ReferenceDescription, UA_TYPES_REFERENCEDESCRIPTION> {
public:
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

public:
    NodeId getReferenceTypeId() const;
    void setReferenceTypeId(const NodeId& referenceTypeId);
    bool isForward() const;
    void setIsForward(bool isForward);
    ExpandedNodeId getNodeId() const;
    void setNodeId(const ExpandedNodeId& nodeId);
    QualifiedName getBrowseName() const;
    void setBrowseName(const QualifiedName& browseName);
    LocalizedText getDisplayName() const;
    void setDisplayName(const LocalizedText& displayName);
    NodeClass getNodeClass() const;
    void setNodeClass(NodeClass nodeClass);
    ExpandedNodeId getTypeDefinition() const;
    void setTypeDefinition(const ExpandedNodeId& typeDefinition);
};

}  // namespace opcua