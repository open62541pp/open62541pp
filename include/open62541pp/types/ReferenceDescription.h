#pragma once

#include "open62541pp/types/NodeId.h"
#include "open62541pp/Common.h"

#include "open62541pp/TypeWrapper.h"
#include "open62541pp/Common.h"
#include "open62541pp/open62541.h"

namespace opcua {

class ReferenceDescription
    : public TypeWrapper<UA_ReferenceDescription, UA_TYPES_REFERENCEDESCRIPTION> {
public:
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

public:
    /**
     * @brief get reference type id
     * @return
     */
    NodeId getReferenceTypeId() const;

    /**
     * @brief set reference type id
     * @param referenceTypeId
     */
    void setReferenceTypeId(const NodeId& referenceTypeId);

    /**
     * @brief get is forward flag
     * @return forward flag
     */
    bool isForward() const;

    /**
     * @brief set is forward flag
     * @param isForward
     */
    void setIsForward(bool isForward);

    /**
     * @brief get node id
     * @return node id
     */
    ExpandedNodeId getNodeId() const;

    /**
     * @brief set node id
     * @param nodeId
     */
    void setNodeId(const ExpandedNodeId& nodeId);

    /**
     * @brief get browse name
     * @return browse name
     */
    QualifiedName getBrowseName() const;

    /**
     * @brief set browse name
     * @param browseName
     */
    void setBrowseName(const QualifiedName& browseName);

    /**
     * @brief get display name
     * @return display name
     */
    LocalizedText getDisplayName() const;

    /**
     * @brief set display name
     * @param displayName
     */
    void setDisplayName(const LocalizedText& displayName);

    /**
     * @brief get node class
     * @return node class
     */
    NodeClass getNodeClass() const;

    /**
     * @brief set node class
     * @param nodeClass
     */
    void setNodeClass(NodeClass nodeClass);

    /**
     * @brief get type definition
     * @return type definition
     */
    ExpandedNodeId getTypeDefinition() const;

    /**
     * @brief set type definition
     * @param typeDefinition
     */
    void setTypeDefinition(const ExpandedNodeId& typeDefinition);
};

}  // namespace opcua