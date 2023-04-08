#include "open62541pp/ReferenceDescription.h"
#include "open62541pp/TypeConverter.h"

namespace opcua {

NodeId ReferenceDescription::getReferenceTypeId() const {
    NodeId ret;
    TypeConverter<UA_NodeId>::fromNative(handle()->referenceTypeId, ret);
    return ret;
}

void ReferenceDescription::setReferenceTypeId(const NodeId& referenceTypeId) {
    TypeConverter<UA_NodeId>::toNative(referenceTypeId, handle()->referenceTypeId);
}

bool ReferenceDescription::isForward() const {
    bool ret;
    TypeConverter<UA_Boolean>::fromNative(handle()->isForward, ret);
    return ret;
}

void ReferenceDescription::setIsForward(bool isForward) {
    TypeConverter<UA_Boolean>::toNative(isForward, handle()->isForward);
}

ExpandedNodeId ReferenceDescription::getNodeId() const {
    ExpandedNodeId ret;
    TypeConverter<UA_ExpandedNodeId>::fromNative(handle()->nodeId, ret);
    return ret;
}

void ReferenceDescription::setNodeId(const ExpandedNodeId& nodeId) {
    TypeConverter<UA_ExpandedNodeId>::toNative(nodeId, handle()->nodeId);
}

QualifiedName ReferenceDescription::getBrowseName() const {
    QualifiedName ret;
    TypeConverter<UA_QualifiedName>::fromNative(handle()->browseName, ret);
    return ret;
}

void ReferenceDescription::setBrowseName(const QualifiedName& browseName) {
    TypeConverter<UA_QualifiedName>::toNative(browseName, handle()->browseName);
}

LocalizedText ReferenceDescription::getDisplayName() const {
    LocalizedText ret;
    TypeConverter<UA_LocalizedText>::fromNative(handle()->displayName, ret);
    return ret;
}

void ReferenceDescription::setDisplayName(const LocalizedText& displayName) {
    TypeConverter<UA_LocalizedText>::toNative(displayName, handle()->displayName);
}

NodeClass ReferenceDescription::getNodeClass() const {
    return static_cast<NodeClass>(handle()->nodeClass);
}

void ReferenceDescription::setNodeClass(NodeClass nodeClass) {
    handle()->nodeClass = static_cast<UA_NodeClass>(nodeClass);
}

ExpandedNodeId ReferenceDescription::getTypeDefinition() const {
    ExpandedNodeId ret;
    TypeConverter<UA_ExpandedNodeId>::fromNative(handle()->typeDefinition, ret);
    return ret;
}

void ReferenceDescription::setTypeDefinition(const ExpandedNodeId& typeDefinition) {
    TypeConverter<UA_ExpandedNodeId>::toNative(typeDefinition, handle()->typeDefinition);
}
}  // namespace opcua