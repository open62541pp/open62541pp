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
    ReferenceDescription(
        const NodeId& referenceTypeId,
        bool isForward,
        const ExpandedNodeId& nodeId,
        const QualifiedName& browseName,
        const LocalizedText& displayName,
        NodeClass nodeClass,
        const ExpandedNodeId& typeDefinition
    );

    /**
     * @brief get reference type id
     * @return
     */
    NodeId const& getReferenceTypeId() const noexcept;
    NodeId& getReferenceTypeId() noexcept;

    /**
     * @brief get is forward flag
     * @return forward flag
     */
    bool const& isForward() const noexcept;
    bool& isForward() noexcept;

    /**
     * @brief get node id
     * @return node id
     */
    ExpandedNodeId const& getNodeId() const noexcept;
    ExpandedNodeId& getNodeId() noexcept;

    /**
     * @brief get browse name
     * @return browse name
     */
    QualifiedName const& getBrowseName() const noexcept;
    QualifiedName& getBrowseName() noexcept;

    /**
     * @brief get display name
     * @return display name
     */
    LocalizedText const& getDisplayName() const noexcept;
    LocalizedText& getDisplayName() noexcept;

    /**
     * @brief get node class
     * @return node class
     */
    NodeClass const& getNodeClass() const noexcept;
    NodeClass& getNodeClass() noexcept;

    /**
     * @brief get type definition
     * @return type definition
     */
    ExpandedNodeId const& getTypeDefinition() const noexcept;
    ExpandedNodeId& getTypeDefinition() noexcept;
};

}  // namespace opcua