#include "open62541pp/types/ReferenceDescription.h"
#include "open62541pp/TypeConverter.h"
#include "open62541pp/TypeWrapper.h"

namespace opcua {

ReferenceDescription::ReferenceDescription(
    const NodeId& referenceTypeId,
    bool isForward,
    const ExpandedNodeId& nodeId,
    const QualifiedName& browseName,
    const LocalizedText& displayName,
    NodeClass nodeClass,
    const ExpandedNodeId& typeDefinition
) {
    TypeConverter<UA_NodeId>::toNative(referenceTypeId, handle()->referenceTypeId);
    TypeConverter<UA_Boolean>::toNative(isForward, handle()->isForward);
    TypeConverter<UA_ExpandedNodeId>::toNative(nodeId, handle()->nodeId);
    TypeConverter<UA_QualifiedName>::toNative(browseName, handle()->browseName);
    TypeConverter<UA_LocalizedText>::toNative(displayName, handle()->displayName);
    handle()->nodeClass = static_cast<UA_NodeClass>(nodeClass);
    TypeConverter<UA_ExpandedNodeId>::toNative(typeDefinition, handle()->typeDefinition);
}

NodeId const& ReferenceDescription::getReferenceTypeId() const noexcept {
    return asWrapper<NodeId>(handle()->referenceTypeId);
}

NodeId& ReferenceDescription::getReferenceTypeId() noexcept {
    return asWrapper<NodeId>(handle()->referenceTypeId);
}

bool const& ReferenceDescription::isForward() const noexcept {
    return asWrapper<bool>(handle()->isForward);
}

bool& ReferenceDescription::isForward() noexcept {
    return asWrapper<bool>(handle()->isForward);
}

ExpandedNodeId const& ReferenceDescription::getNodeId() const noexcept {
    return asWrapper<ExpandedNodeId>(handle()->nodeId);
}

ExpandedNodeId& ReferenceDescription::getNodeId() noexcept {
    return asWrapper<ExpandedNodeId>(handle()->nodeId);
}

QualifiedName const& ReferenceDescription::getBrowseName() const noexcept {
    return asWrapper<QualifiedName>(handle()->browseName);
}

QualifiedName& ReferenceDescription::getBrowseName() noexcept {
    return asWrapper<QualifiedName>(handle()->browseName);
}

LocalizedText const& ReferenceDescription::getDisplayName() const noexcept {
    return asWrapper<LocalizedText>(handle()->displayName);
}

LocalizedText& ReferenceDescription::getDisplayName() noexcept {
    return asWrapper<LocalizedText>(handle()->displayName);
}

NodeClass const& ReferenceDescription::getNodeClass() const noexcept {
    return asWrapper<NodeClass>(handle()->nodeClass);
}

NodeClass& ReferenceDescription::getNodeClass() noexcept {
    return asWrapper<NodeClass>(handle()->nodeClass);
}

ExpandedNodeId const& ReferenceDescription::getTypeDefinition() const noexcept {
    return asWrapper<ExpandedNodeId>(handle()->typeDefinition);
}

ExpandedNodeId& ReferenceDescription::getTypeDefinition() noexcept {
    return asWrapper<ExpandedNodeId>(handle()->typeDefinition);
}

}  // namespace opcua