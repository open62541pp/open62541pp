#include "open62541pp/types/Composed.h"

#include <algorithm>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/detail/helper.h"

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

#ifdef UA_ENABLE_METHODCALLS

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
    copyArray(arrayDimensions, &handle()->arrayDimensions, handle()->arrayDimensionsSize);
}

#endif

#ifdef UA_ENABLE_SUBSCRIPTIONS

ElementOperand::ElementOperand(uint32_t index) {
    handle()->index = index;
}

LiteralOperand::LiteralOperand(const Variant& value) {
    asWrapper<Variant>(handle()->value) = value;
}

AttributeOperand::AttributeOperand(
    const NodeId& nodeId,
    std::string_view alias,
    const RelativePath& browsePath,
    AttributeId attributeId,
    [[maybe_unused]] std::string_view indexRange
) {
    asWrapper<NodeId>(handle()->nodeId) = nodeId;
    asWrapper<String>(handle()->alias) = String(alias);
    asWrapper<RelativePath>(handle()->browsePath) = browsePath;
    handle()->attributeId = static_cast<uint32_t>(attributeId);
    asWrapper<String>(handle()->indexRange) = String(indexRange);
}

SimpleAttributeOperand::SimpleAttributeOperand(
    const NodeId& typeDefinitionId,
    Span<const QualifiedName> browsePath,
    AttributeId attributeId,
    [[maybe_unused]] std::string_view indexRange
) {
    asWrapper<NodeId>(handle()->typeDefinitionId) = typeDefinitionId;
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
            // TODO: non-const get* method to return span
            auto operands = Span(
                asWrapper<ExtensionObject>(element->filterOperands), element->filterOperandsSize
            );
            for (auto& operand : operands) {
                auto* elementOperand = operand.getDecodedData<ElementOperand>();
                if (elementOperand != nullptr) {
                    elementOperand->handle()->index += offset;
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
        {{binaryOperator, {ElementOperand(1), ElementOperand(1 + lhs.size())}}},
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
    Span<const SimpleAttributeOperand> selectClauses, const ContentFilter& whereClause
) {
    copyArray(selectClauses, &handle()->selectClauses, handle()->selectClausesSize);
    asWrapper<ContentFilter>(handle()->whereClause) = whereClause;
}

AggregateFilter::AggregateFilter(
    DateTime startTime,
    const NodeId& aggregateType,
    double processingInterval,
    AggregateConfiguration aggregateConfiguration
) {
    asWrapper<DateTime>(handle()->startTime) = std::move(startTime);
    asWrapper<NodeId>(handle()->aggregateType) = aggregateType;
    handle()->processingInterval = processingInterval;
    handle()->aggregateConfiguration = aggregateConfiguration;
}

#endif

}  // namespace opcua
