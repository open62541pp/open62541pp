#include "open62541pp/ua/types.hpp"

#include <algorithm>  // copy, transform

namespace opcua {
inline namespace ua {

#ifdef UA_ENABLE_SUBSCRIPTIONS

ContentFilterElement::ContentFilterElement(
    FilterOperator filterOperator, Span<const FilterOperand> operands
) {
    handle()->filterOperator = static_cast<UA_FilterOperator>(filterOperator);
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
    : ContentFilter({elements.begin(), elements.size()}) {}

ContentFilter::ContentFilter(Span<const ContentFilterElement> elements) {
    handle()->elementsSize = elements.size();
    handle()->elements = detail::toNativeArray(elements);
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

    const Span<ContentFilterElement> resultElements(
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

static ContentFilter applyUnaryOperator(
    FilterOperator unaryOperator, Span<const ContentFilterElement> elements
) {
    return concatFilterElements({
        {{unaryOperator, {ElementOperand(1)}}},
        elements,
    });
}

static ContentFilter applyBinaryOperator(
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

#endif

}  // namespace ua
}  // namespace opcua
