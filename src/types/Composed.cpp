#include "open62541pp/types/Composed.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <string_view>
#include <type_traits>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/helper.h"

#include "../open62541_impl.h"

namespace opcua {

template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
inline static void assign(T src, T& dst) noexcept {
    dst = src;
}

template <typename T, typename Native, typename = std::enable_if_t<std::is_enum_v<T>>>
inline static void assign(T src, Native& dst) noexcept {
    dst = static_cast<Native>(src);
}

inline static void assign(std::string_view src, UA_String& dst) {
    // UA_String is empty in constructor call, no clear necessary
    assert(dst.data == nullptr);
    dst = detail::allocNativeString(src);
}

template <typename T, typename Native, typename = std::enable_if_t<detail::isTypeWrapper<T>>>
inline static void assign(T&& src, Native& dst) {
    static_assert(std::is_same_v<typename T::NativeType, Native>);
    asWrapper<T>(dst) = std::forward<T>(src);
}

template <typename T, typename Native>
static void assignArray(Span<const T> src, Native*& dst, size_t& dstSize) {
    static_assert(sizeof(T) == sizeof(Native));
    if constexpr (detail::isTypeWrapper<T>) {
        dst = detail::copyArray(asNative(src.data()), src.size(), detail::getDataType<T>());
    } else {
        dst = detail::copyArray(src.data(), src.size(), detail::getDataType<T>());
    }
    dstSize = src.size();
}

RequestHeader::RequestHeader(
    NodeId authenticationToken,
    DateTime timestamp,
    uint32_t requestHandle,
    uint32_t returnDiagnostics,
    std::string_view auditEntryId,
    uint32_t timeoutHint,
    ExtensionObject additionalHeader
) {
    assign(std::move(authenticationToken), handle()->authenticationToken);
    assign(std::move(timestamp), handle()->timestamp);
    assign(requestHandle, handle()->requestHandle);
    assign(returnDiagnostics, handle()->returnDiagnostics);
    assign(auditEntryId, handle()->auditEntryId);
    assign(timeoutHint, handle()->timeoutHint);
    assign(std::move(additionalHeader), handle()->additionalHeader);
}

UserTokenPolicy::UserTokenPolicy(
    std::string_view policyId,
    UserTokenType tokenType,
    std::string_view issuedTokenType,
    std::string_view issuerEndpointUrl,
    std::string_view securityPolicyUri
) {
    assign(policyId, handle()->policyId);
    assign(tokenType, handle()->tokenType);
    assign(issuedTokenType, handle()->issuedTokenType);
    assign(issuerEndpointUrl, handle()->issuerEndpointUrl);
    assign(securityPolicyUri, handle()->securityPolicyUri);
}

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

AddNodesItem::AddNodesItem(
    ExpandedNodeId parentNodeId,
    NodeId referenceTypeId,
    ExpandedNodeId requestedNewNodeId,
    QualifiedName browseName,
    NodeClass nodeClass,
    ExtensionObject nodeAttributes,
    ExpandedNodeId typeDefinition
) {
    assign(std::move(parentNodeId), handle()->parentNodeId);
    assign(std::move(referenceTypeId), handle()->referenceTypeId);
    assign(std::move(requestedNewNodeId), handle()->requestedNewNodeId);
    assign(std::move(browseName), handle()->browseName);
    assign(nodeClass, handle()->nodeClass);
    assign(std::move(nodeAttributes), handle()->nodeAttributes);
    assign(std::move(typeDefinition), handle()->typeDefinition);
}

AddNodesRequest::AddNodesRequest(RequestHeader requestHeader, Span<const AddNodesItem> nodesToAdd) {
    assign(std::move(requestHeader), handle()->requestHeader);
    assignArray(nodesToAdd, handle()->nodesToAdd, handle()->nodesToAddSize);
}

AddReferencesItem::AddReferencesItem(
    NodeId sourceNodeId,
    NodeId referenceTypeId,
    bool isForward,
    std::string_view targetServerUri,
    ExpandedNodeId targetNodeId,
    NodeClass targetNodeClass
) {
    assign(std::move(sourceNodeId), handle()->sourceNodeId);
    assign(std::move(referenceTypeId), handle()->referenceTypeId);
    assign(isForward, handle()->isForward);
    assign(targetServerUri, handle()->targetServerUri);
    assign(std::move(targetNodeId), handle()->targetNodeId);
    assign(targetNodeClass, handle()->targetNodeClass);
}

AddReferencesRequest::AddReferencesRequest(
    RequestHeader requestHeader, Span<const AddReferencesItem> referencesToAdd
) {
    assign(std::move(requestHeader), handle()->requestHeader);
    assignArray(referencesToAdd, handle()->referencesToAdd, handle()->referencesToAddSize);
}

DeleteNodesItem::DeleteNodesItem(NodeId nodeId, bool deleteTargetReferences) {
    assign(std::move(nodeId), handle()->nodeId);
    assign(deleteTargetReferences, handle()->deleteTargetReferences);
}

DeleteNodesRequest::DeleteNodesRequest(
    RequestHeader requestHeader, Span<const DeleteNodesItem> nodesToDelete
) {
    assign(std::move(requestHeader), handle()->requestHeader);
    assignArray(nodesToDelete, handle()->nodesToDelete, handle()->nodesToDeleteSize);
}

DeleteReferencesItem::DeleteReferencesItem(
    NodeId sourceNodeId,
    NodeId referenceTypeId,
    bool isForward,
    ExpandedNodeId targetNodeId,
    bool deleteBidirectional
) {
    assign(std::move(sourceNodeId), handle()->sourceNodeId);
    assign(std::move(referenceTypeId), handle()->referenceTypeId);
    assign(isForward, handle()->isForward);
    assign(std::move(targetNodeId), handle()->targetNodeId);
    assign(deleteBidirectional, handle()->deleteBidirectional);
}

DeleteReferencesRequest::DeleteReferencesRequest(
    RequestHeader requestHeader, Span<const DeleteReferencesItem> referencesToDelete
) {
    assign(std::move(requestHeader), handle()->requestHeader);
    assignArray(referencesToDelete, handle()->referencesToDelete, handle()->referencesToDeleteSize);
}

ViewDescription::ViewDescription(NodeId viewId, DateTime timestamp, uint32_t viewVersion) {
    assign(std::move(viewId), handle()->viewId);
    assign(std::move(timestamp), handle()->timestamp);
    assign(viewVersion, handle()->viewVersion);
}

BrowseDescription::BrowseDescription(
    NodeId nodeId,
    BrowseDirection browseDirection,
    NodeId referenceTypeId,
    bool includeSubtypes,
    uint32_t nodeClassMask,
    uint32_t resultMask
) {
    assign(std::move(nodeId), handle()->nodeId);
    assign(browseDirection, handle()->browseDirection);
    assign(std::move(referenceTypeId), handle()->referenceTypeId);
    assign(includeSubtypes, handle()->includeSubtypes);
    assign(nodeClassMask, handle()->nodeClassMask);
    assign(resultMask, handle()->resultMask);
}

RelativePathElement::RelativePathElement(
    NodeId referenceTypeId, bool isInverse, bool includeSubtypes, QualifiedName targetName
) {
    assign(std::move(referenceTypeId), handle()->referenceTypeId);
    assign(isInverse, handle()->isInverse);
    assign(includeSubtypes, handle()->includeSubtypes);
    assign(std::move(targetName), handle()->targetName);
}

RelativePath::RelativePath(std::initializer_list<RelativePathElement> elements)
    : RelativePath({elements.begin(), elements.size()}) {}

RelativePath::RelativePath(Span<const RelativePathElement> elements) {
    assignArray(elements, handle()->elements, handle()->elementsSize);
}

BrowsePath::BrowsePath(NodeId startingNode, RelativePath relativePath) {
    assign(std::move(startingNode), handle()->startingNode);
    assign(std::move(relativePath), handle()->relativePath);
}

BrowseRequest::BrowseRequest(
    RequestHeader requestHeader,
    ViewDescription view,
    uint32_t requestedMaxReferencesPerNode,
    Span<const BrowseDescription> nodesToBrowse
) {
    assign(std::move(requestHeader), handle()->requestHeader);
    assign(std::move(view), handle()->view);
    assign(requestedMaxReferencesPerNode, handle()->requestedMaxReferencesPerNode);
    assignArray(nodesToBrowse, handle()->nodesToBrowse, handle()->nodesToBrowseSize);
}

BrowseNextRequest::BrowseNextRequest(
    RequestHeader requestHeader,
    bool releaseContinuationPoints,
    Span<const ByteString> continuationPoints
) {
    assign(std::move(requestHeader), handle()->requestHeader);
    assign(releaseContinuationPoints, handle()->releaseContinuationPoints);
    assignArray(continuationPoints, handle()->continuationPoints, handle()->continuationPointsSize);
}

TranslateBrowsePathsToNodeIdsRequest::TranslateBrowsePathsToNodeIdsRequest(
    RequestHeader requestHeader, Span<const BrowsePath> browsePaths
) {
    assign(std::move(requestHeader), handle()->requestHeader);
    assignArray(browsePaths, handle()->browsePaths, handle()->browsePathsSize);
}

RegisterNodesRequest::RegisterNodesRequest(
    RequestHeader requestHeader, Span<const NodeId> nodesToRegister
) {
    assign(std::move(requestHeader), handle()->requestHeader);
    assignArray(nodesToRegister, handle()->nodesToRegister, handle()->nodesToRegisterSize);
}

UnregisterNodesRequest::UnregisterNodesRequest(
    RequestHeader requestHeader, Span<const NodeId> nodesToUnregister
) {
    assign(std::move(requestHeader), handle()->requestHeader);
    assignArray(nodesToUnregister, handle()->nodesToUnregister, handle()->nodesToUnregisterSize);
}

ReadValueId::ReadValueId(
    NodeId nodeId, AttributeId attributeId, std::string_view indexRange, QualifiedName dataEncoding
) {
    assign(std::move(nodeId), handle()->nodeId);
    assign(attributeId, handle()->attributeId);
    assign(indexRange, handle()->indexRange);
    assign(std::move(dataEncoding), handle()->dataEncoding);
}

ReadRequest::ReadRequest(
    RequestHeader requestHeader,
    double maxAge,
    TimestampsToReturn timestampsToReturn,
    Span<const ReadValueId> nodesToRead
) {
    assign(std::move(requestHeader), handle()->requestHeader);
    assign(maxAge, handle()->maxAge);
    assign(timestampsToReturn, handle()->timestampsToReturn);
    assignArray(nodesToRead, handle()->nodesToRead, handle()->nodesToReadSize);
}

WriteValue::WriteValue(
    NodeId nodeId, AttributeId attributeId, std::string_view indexRange, DataValue value
) {
    assign(std::move(nodeId), handle()->nodeId);
    assign(attributeId, handle()->attributeId);
    assign(indexRange, handle()->indexRange);
    assign(std::move(value), handle()->value);
}

WriteRequest::WriteRequest(RequestHeader requestHeader, Span<const WriteValue> nodesToWrite) {
    assign(std::move(requestHeader), handle()->requestHeader);
    assignArray(nodesToWrite, handle()->nodesToWrite, handle()->nodesToWriteSize);
}

EnumValueType::EnumValueType(int64_t value, LocalizedText displayName, LocalizedText description) {
    assign(value, handle()->value);
    assign(std::move(displayName), handle()->displayName);
    assign(std::move(description), handle()->description);
}

#ifdef UA_ENABLE_METHODCALLS

Argument::Argument(
    std::string_view name,
    LocalizedText description,
    NodeId dataType,
    ValueRank valueRank,
    Span<const uint32_t> arrayDimensions
) {
    assign(name, handle()->name);
    assign(std::move(description), handle()->description);
    assign(std::move(dataType), handle()->dataType);
    assign(valueRank, handle()->valueRank);
    assignArray(arrayDimensions, handle()->arrayDimensions, handle()->arrayDimensionsSize);
}

CallMethodRequest::CallMethodRequest(
    NodeId objectId, NodeId methodId, Span<const Variant> inputArguments
) {
    assign(std::move(objectId), handle()->objectId);
    assign(std::move(methodId), handle()->methodId);
    assignArray(inputArguments, handle()->inputArguments, handle()->inputArgumentsSize);
}

CallRequest::CallRequest(RequestHeader requestHeader, Span<const CallMethodRequest> methodsToCall) {
    assign(std::move(requestHeader), handle()->requestHeader);
    assignArray(methodsToCall, handle()->methodsToCall, handle()->methodsToCallSize);
}

#endif

#ifdef UA_ENABLE_SUBSCRIPTIONS

ElementOperand::ElementOperand(uint32_t index) {
    assign(index, handle()->index);
}

LiteralOperand::LiteralOperand(Variant value) {
    assign(std::move(value), handle()->value);
}

AttributeOperand::AttributeOperand(
    NodeId nodeId,
    std::string_view alias,
    RelativePath browsePath,
    AttributeId attributeId,
    [[maybe_unused]] std::string_view indexRange
) {
    assign(std::move(nodeId), handle()->nodeId);
    assign(alias, handle()->alias);
    assign(std::move(browsePath), handle()->browsePath);
    assign(attributeId, handle()->attributeId);
    assign(indexRange, handle()->indexRange);
}

SimpleAttributeOperand::SimpleAttributeOperand(
    NodeId typeDefinitionId,
    Span<const QualifiedName> browsePath,
    AttributeId attributeId,
    [[maybe_unused]] std::string_view indexRange
) {
    assign(std::move(typeDefinitionId), handle()->typeDefinitionId);
    assignArray(browsePath, handle()->browsePath, handle()->browsePathSize);
    assign(attributeId, handle()->attributeId);
    assign(indexRange, handle()->indexRange);
}

ContentFilterElement::ContentFilterElement(
    FilterOperator filterOperator, Span<const FilterOperand> operands
) {
    assign(filterOperator, handle()->filterOperator);
    handle()->filterOperandsSize = operands.size();
    handle()->filterOperands = detail::allocateArray<UA_ExtensionObject>(
        operands.size(), UA_TYPES[UA_TYPES_EXTENSIONOBJECT]
    );

    // transform array of operand variants to array of extension objects
    std::transform(
        operands.begin(),
        operands.end(),
        asWrapper<ExtensionObject>(handle()->filterOperands),
        [](auto&& variant) {
            return std::visit(
                [](auto&& operand) { return ExtensionObject::fromDecodedCopy(operand); }, variant
            );
        }
    );
}

ContentFilter::ContentFilter(std::initializer_list<ContentFilterElement> elements)
    : ContentFilter(Span<const ContentFilterElement>(elements)) {}

ContentFilter::ContentFilter(Span<const ContentFilterElement> elements) {
    assignArray(elements, handle()->elements, handle()->elementsSize);
}

/* ----------------------------------- ContentFilter operators ---------------------------------- */

static ContentFilter concatFilterElements(
    std::initializer_list<Span<const ContentFilterElement>> filterElementsList
) {
    size_t totalSize = 0;
    for (const auto& filterElements : filterElementsList) {
        totalSize += filterElements.size();
    }

    ContentFilter result;
    result->elementsSize = totalSize;
    result->elements = detail::allocateArray<UA_ContentFilterElement>(
        totalSize, UA_TYPES[UA_TYPES_CONTENTFILTERELEMENT]
    );

    Span<ContentFilterElement> resultElements(
        asWrapper<ContentFilterElement>(result->elements), totalSize
    );

    size_t offset = 0;
    for (const auto& filterElements : filterElementsList) {
        auto resultElementsView = resultElements.subview(offset, filterElements.size());
        // copy to result
        std::copy(filterElements.begin(), filterElements.end(), resultElementsView.begin());
        // increment element operand indexes by offset
        for (auto& element : resultElementsView) {
            for (auto& operand : element.getFilterOperands()) {
                auto* elementOperand = operand.getDecodedData<ElementOperand>();
                if (elementOperand != nullptr) {
                    elementOperand->handle()->index += static_cast<uint32_t>(offset);
                }
            }
        }
        // increment offset
        offset += filterElements.size();
    }
    return result;
}

inline static ContentFilter applyUnaryOperator(
    FilterOperator unaryOperator, Span<const ContentFilterElement> elements
) {
    return concatFilterElements({
        {{unaryOperator, {ElementOperand(1)}}},
        elements,
    });
}

inline static ContentFilter applyBinaryOperator(
    FilterOperator binaryOperator,
    Span<const ContentFilterElement> lhs,
    Span<const ContentFilterElement> rhs
) {
    return concatFilterElements({
        {{
            binaryOperator,
            {
                ElementOperand(1),
                ElementOperand(1 + static_cast<uint32_t>(lhs.size())),
            },
        }},
        lhs,
        rhs,
    });
}

ContentFilter operator!(const ContentFilterElement& filterElement) {
    return applyUnaryOperator(FilterOperator::Not, {filterElement});
}

ContentFilter operator!(const ContentFilter& filter) {
    return applyUnaryOperator(FilterOperator::Not, filter.getElements());
}

ContentFilter operator&&(const ContentFilterElement& lhs, const ContentFilterElement& rhs) {
    return applyBinaryOperator(FilterOperator::And, {lhs}, {rhs});
}

ContentFilter operator&&(const ContentFilterElement& lhs, const ContentFilter& rhs) {
    return applyBinaryOperator(FilterOperator::And, {lhs}, rhs.getElements());
}

ContentFilter operator&&(const ContentFilter& lhs, const ContentFilterElement& rhs) {
    return applyBinaryOperator(FilterOperator::And, lhs.getElements(), {rhs});
}

ContentFilter operator&&(const ContentFilter& lhs, const ContentFilter& rhs) {
    return applyBinaryOperator(FilterOperator::And, lhs.getElements(), rhs.getElements());
}

ContentFilter operator||(const ContentFilterElement& lhs, const ContentFilterElement& rhs) {
    return applyBinaryOperator(FilterOperator::Or, {lhs}, {rhs});
}

ContentFilter operator||(const ContentFilterElement& lhs, const ContentFilter& rhs) {
    return applyBinaryOperator(FilterOperator::Or, {lhs}, rhs.getElements());
}

ContentFilter operator||(const ContentFilter& lhs, const ContentFilterElement& rhs) {
    return applyBinaryOperator(FilterOperator::Or, lhs.getElements(), {rhs});
}

ContentFilter operator||(const ContentFilter& lhs, const ContentFilter& rhs) {
    return applyBinaryOperator(FilterOperator::Or, lhs.getElements(), rhs.getElements());
}

/* ---------------------------------------------------------------------------------------------- */

DataChangeFilter::DataChangeFilter(
    DataChangeTrigger trigger, DeadbandType deadbandType, double deadbandValue
) {
    assign(trigger, handle()->trigger);
    assign(deadbandType, handle()->deadbandType);
    assign(deadbandValue, handle()->deadbandValue);
}

EventFilter::EventFilter(
    Span<const SimpleAttributeOperand> selectClauses, ContentFilter whereClause
) {
    assignArray(selectClauses, handle()->selectClauses, handle()->selectClausesSize);
    assign(std::move(whereClause), handle()->whereClause);
}

AggregateFilter::AggregateFilter(
    DateTime startTime,
    NodeId aggregateType,
    double processingInterval,
    AggregateConfiguration aggregateConfiguration
) {
    assign(std::move(startTime), handle()->startTime);
    assign(std::move(aggregateType), handle()->aggregateType);
    assign(processingInterval, handle()->processingInterval);
    assign(aggregateConfiguration, handle()->aggregateConfiguration);  // TODO: make wrapper?
}

#endif

}  // namespace opcua
