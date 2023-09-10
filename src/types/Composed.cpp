#include "open62541pp/types/Composed.h"

#include <algorithm>
#include <cstddef>

#include "open62541pp/ErrorHandling.h"

#include "../open62541_impl.h"

namespace opcua {

template <typename T, typename Native>
static void copyArray(Span<const T> src, Native** dst, size_t& dstSize) {
    static_assert(sizeof(T) == sizeof(Native));
    dstSize = src.size();
    const auto status = UA_Array_copy(
        src.data(),
        src.size(),
        (void**)dst,  // NOLINT
        &detail::guessDataType<T>()
    );
    detail::throwOnBadStatus(status);
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
    asWrapper<NodeId>(handle()->authenticationToken) = std::move(authenticationToken);
    handle()->timestamp = timestamp;
    handle()->requestHandle = requestHandle;
    handle()->returnDiagnostics = returnDiagnostics;
    asWrapper<String>(handle()->auditEntryId) = String(auditEntryId);
    handle()->timeoutHint = timeoutHint;
    asWrapper<ExtensionObject>(handle()->additionalHeader) = std::move(additionalHeader);
}

UserTokenPolicy::UserTokenPolicy(
    std::string_view policyId,
    UserTokenType tokenType,
    std::string_view issuedTokenType,
    std::string_view issuerEndpointUrl,
    std::string_view securityPolicyUri
) {
    asWrapper<String>(handle()->policyId) = String(policyId);
    handle()->tokenType = static_cast<UA_UserTokenType>(tokenType);
    asWrapper<String>(handle()->issuedTokenType) = String(issuedTokenType);
    asWrapper<String>(handle()->issuerEndpointUrl) = String(issuerEndpointUrl);
    asWrapper<String>(handle()->securityPolicyUri) = String(securityPolicyUri);
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

BrowseDescription::BrowseDescription(
    NodeId nodeId,
    BrowseDirection browseDirection,
    NodeId referenceType,
    bool includeSubtypes,
    uint32_t nodeClassMask,
    uint32_t resultMask
) {
    asWrapper<NodeId>(handle()->nodeId) = std::move(nodeId);
    handle()->browseDirection = static_cast<UA_BrowseDirection>(browseDirection);
    asWrapper<NodeId>(handle()->referenceTypeId) = std::move(referenceType);
    handle()->includeSubtypes = includeSubtypes;
    handle()->nodeClassMask = nodeClassMask;
    handle()->resultMask = resultMask;
}

RelativePathElement::RelativePathElement(
    NodeId referenceType, bool isInverse, bool includeSubtypes, QualifiedName targetName
) {
    asWrapper<NodeId>(handle()->referenceTypeId) = std::move(referenceType);
    handle()->isInverse = isInverse;
    handle()->includeSubtypes = includeSubtypes;
    asWrapper<QualifiedName>(handle()->targetName) = std::move(targetName);
}

RelativePath::RelativePath(std::initializer_list<RelativePathElement> elements) {
    handle()->elementsSize = elements.size();
    handle()->elements = detail::toNativeArrayAlloc(elements.begin(), elements.end());
}

RelativePath::RelativePath(Span<const RelativePathElement> elements) {
    handle()->elementsSize = elements.size();
    handle()->elements = detail::toNativeArrayAlloc(elements.begin(), elements.end());
}

BrowsePath::BrowsePath(NodeId startingNode, RelativePath relativePath) {
    asWrapper<NodeId>(handle()->startingNode) = std::move(startingNode);
    asWrapper<RelativePath>(handle()->relativePath) = std::move(relativePath);
}

ReadValueId::ReadValueId(
    NodeId nodeId, AttributeId attributeId, std::string_view indexRange, QualifiedName dataEncoding
) {
    asWrapper<NodeId>(handle()->nodeId) = std::move(nodeId);
    handle()->attributeId = static_cast<uint32_t>(attributeId);
    asWrapper<String>(handle()->indexRange) = String(indexRange);
    asWrapper<QualifiedName>(handle()->dataEncoding) = std::move(dataEncoding);
}

ReadRequest::ReadRequest(
    RequestHeader requestHeader,
    double maxAge,
    TimestampsToReturn timestampsToReturn,
    Span<const ReadValueId> nodesToRead
) {
    asWrapper<RequestHeader>(handle()->requestHeader) = std::move(requestHeader);
    handle()->maxAge = maxAge;
    handle()->timestampsToReturn = static_cast<UA_TimestampsToReturn>(timestampsToReturn);
    copyArray(nodesToRead, &handle()->nodesToRead, handle()->nodesToReadSize);
}

WriteValue::WriteValue(
    NodeId nodeId, AttributeId attributeId, std::string_view indexRange, DataValue value
) {
    asWrapper<NodeId>(handle()->nodeId) = std::move(nodeId);
    handle()->attributeId = static_cast<uint32_t>(attributeId);
    asWrapper<String>(handle()->indexRange) = String(indexRange);
    asWrapper<DataValue>(handle()->value) = std::move(value);
}

WriteRequest::WriteRequest(RequestHeader requestHeader, Span<const WriteValue> nodesToWrite) {
    asWrapper<RequestHeader>(handle()->requestHeader) = std::move(requestHeader);
    copyArray(nodesToWrite, &handle()->nodesToWrite, handle()->nodesToWriteSize);
}

#ifdef UA_ENABLE_METHODCALLS

Argument::Argument(
    std::string_view name,
    LocalizedText description,
    NodeId dataType,
    ValueRank valueRank,
    Span<const uint32_t> arrayDimensions
) {
    asWrapper<String>(handle()->name) = String(name);
    asWrapper<LocalizedText>(handle()->description) = std::move(description);
    asWrapper<NodeId>(handle()->dataType) = std::move(dataType);
    handle()->valueRank = static_cast<UA_Int32>(valueRank);
    copyArray(arrayDimensions, &handle()->arrayDimensions, handle()->arrayDimensionsSize);
}

#endif

#ifdef UA_ENABLE_SUBSCRIPTIONS

ElementOperand::ElementOperand(uint32_t index) {
    handle()->index = index;
}

LiteralOperand::LiteralOperand(Variant value) {
    asWrapper<Variant>(handle()->value) = std::move(value);
}

AttributeOperand::AttributeOperand(
    NodeId nodeId,
    std::string_view alias,
    RelativePath browsePath,
    AttributeId attributeId,
    [[maybe_unused]] std::string_view indexRange
) {
    asWrapper<NodeId>(handle()->nodeId) = std::move(nodeId);
    asWrapper<String>(handle()->alias) = String(alias);
    asWrapper<RelativePath>(handle()->browsePath) = std::move(browsePath);
    handle()->attributeId = static_cast<uint32_t>(attributeId);
    asWrapper<String>(handle()->indexRange) = String(indexRange);
}

SimpleAttributeOperand::SimpleAttributeOperand(
    NodeId typeDefinitionId,
    Span<const QualifiedName> browsePath,
    AttributeId attributeId,
    [[maybe_unused]] std::string_view indexRange
) {
    asWrapper<NodeId>(handle()->typeDefinitionId) = std::move(typeDefinitionId);
    copyArray(browsePath, &handle()->browsePath, handle()->browsePathSize);
    handle()->attributeId = static_cast<uint32_t>(attributeId);
    asWrapper<String>(handle()->indexRange) = String(indexRange);
}

ContentFilterElement::ContentFilterElement(
    FilterOperator filterOperator, Span<const FilterOperand> operands
) {
    handle()->filterOperator = static_cast<UA_FilterOperator>(filterOperator);
    handle()->filterOperandsSize = operands.size();
    handle()->filterOperands = static_cast<UA_ExtensionObject*>(
        UA_Array_new(operands.size(), &UA_TYPES[UA_TYPES_EXTENSIONOBJECT])
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
    copyArray(elements, &handle()->elements, handle()->elementsSize);
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
    result->elements = static_cast<UA_ContentFilterElement*>(
        UA_Array_new(totalSize, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENT])
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
    handle()->trigger = static_cast<UA_DataChangeTrigger>(trigger);
    handle()->deadbandType = static_cast<UA_DeadbandType>(deadbandType);
    handle()->deadbandValue = deadbandValue;
}

EventFilter::EventFilter(
    Span<const SimpleAttributeOperand> selectClauses, ContentFilter whereClause
) {
    copyArray(selectClauses, &handle()->selectClauses, handle()->selectClausesSize);
    asWrapper<ContentFilter>(handle()->whereClause) = std::move(whereClause);
}

AggregateFilter::AggregateFilter(
    DateTime startTime,
    NodeId aggregateType,
    double processingInterval,
    AggregateConfiguration aggregateConfiguration
) {
    handle()->startTime = startTime;
    asWrapper<NodeId>(handle()->aggregateType) = std::move(aggregateType);
    handle()->processingInterval = processingInterval;
    handle()->aggregateConfiguration = aggregateConfiguration;
}

#endif

}  // namespace opcua
