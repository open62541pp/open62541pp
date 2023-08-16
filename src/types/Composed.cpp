#include "open62541pp/types/Composed.h"

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/detail/helper.h"

#include "../open62541_impl.h"

namespace opcua {

ObjectAttributes::ObjectAttributes()
    : TypeWrapperBase(UA_ObjectAttributes_default) {}

VariableAttributes::VariableAttributes()
    : TypeWrapperBase(UA_VariableAttributes_default) {}

MethodAttributes::MethodAttributes()
    : TypeWrapperBase(UA_MethodAttributes_default) {}

ObjectTypeAttributes::ObjectTypeAttributes()
    : TypeWrapperBase(UA_ObjectTypeAttributes_default) {}

VariableTypeAttributes::VariableTypeAttributes()
    : TypeWrapperBase(UA_VariableTypeAttributes_default) {}

ReferenceTypeAttributes::ReferenceTypeAttributes()
    : TypeWrapperBase(UA_ReferenceTypeAttributes_default) {}

DataTypeAttributes::DataTypeAttributes()
    : TypeWrapperBase(UA_DataTypeAttributes_default) {}

ViewAttributes::ViewAttributes()
    : TypeWrapperBase(UA_ViewAttributes_default) {}

BrowseDescription::BrowseDescription(
    const NodeId& nodeId,
    BrowseDirection browseDirection,
    const NodeId& referenceType,
    bool includeSubtypes,
    uint32_t nodeClassMask,
    uint32_t resultMask
) {
    asWrapper<NodeId>(handle()->nodeId) = nodeId;
    handle()->browseDirection = static_cast<UA_BrowseDirection>(browseDirection);
    asWrapper<NodeId>(handle()->referenceTypeId) = referenceType;
    handle()->includeSubtypes = includeSubtypes;
    handle()->nodeClassMask = nodeClassMask;
    handle()->resultMask = resultMask;
}

RelativePathElement::RelativePathElement(
    const NodeId& referenceType,
    bool isInverse,
    bool includeSubtypes,
    const QualifiedName& targetName
) {
    asWrapper<NodeId>(handle()->referenceTypeId) = referenceType;
    handle()->isInverse = isInverse;
    handle()->includeSubtypes = includeSubtypes;
    asWrapper<QualifiedName>(handle()->targetName) = targetName;
}

RelativePath::RelativePath(std::initializer_list<RelativePathElement> elements) {
    handle()->elementsSize = elements.size();
    handle()->elements = detail::toNativeArrayAlloc(elements.begin(), elements.end());
}

RelativePath::RelativePath(Span<const RelativePathElement> elements) {
    handle()->elementsSize = elements.size();
    handle()->elements = detail::toNativeArrayAlloc(elements.begin(), elements.end());
}

BrowsePath::BrowsePath(const NodeId& startingNode, const RelativePath& relativePath) {
    asWrapper<NodeId>(handle()->startingNode) = startingNode;
    asWrapper<RelativePath>(handle()->relativePath) = relativePath;
}

ReadValueId::ReadValueId(const NodeId& id, AttributeId attributeId) {
    asWrapper<NodeId>(handle()->nodeId) = id;
    handle()->attributeId = static_cast<uint32_t>(attributeId);
}

#ifdef UA_TYPES_ARGUMENT
Argument::Argument(
    std::string_view name,
    const LocalizedText& description,
    const NodeId& dataType,
    ValueRank valueRank,
    Span<const uint32_t> arrayDimensions
) {
    asWrapper<String>(handle()->name) = String(name);
    asWrapper<LocalizedText>(handle()->description) = description;
    asWrapper<NodeId>(handle()->dataType) = dataType;
    handle()->valueRank = static_cast<UA_Int32>(valueRank);
    handle()->arrayDimensionsSize = arrayDimensions.size();
    const auto status = UA_Array_copy(
        arrayDimensions.data(),
        arrayDimensions.size(),
        (void**)&handle()->arrayDimensions,  // NOLINT
        &detail::getUaDataType(UA_TYPES_UINT32)
    );
    detail::throwOnBadStatus(status);
}
#endif

}  // namespace opcua
