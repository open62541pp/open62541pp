#include <array>
#include <list>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "open62541pp/detail/traits.hpp"
#include "open62541pp/span.hpp"

using namespace opcua;

TEST_CASE("RangeValueT") {
    STATIC_CHECK(std::is_same_v<detail::RangeValueT<std::vector<int>>, int>);
    STATIC_CHECK(std::is_same_v<detail::RangeValueT<std::vector<int>&>, int>);
    STATIC_CHECK(std::is_same_v<detail::RangeValueT<std::vector<int>&&>, int>);
    STATIC_CHECK(std::is_same_v<detail::RangeValueT<const std::vector<int>>, int>);
    STATIC_CHECK(std::is_same_v<detail::RangeValueT<const std::vector<int>&>, int>);
    STATIC_CHECK(std::is_same_v<detail::RangeValueT<const std::vector<int>&&>, int>);

    STATIC_CHECK(std::is_same_v<detail::RangeValueT<int[3]>, int>);  // NOLINT(*avoid-c-arrays)
    STATIC_CHECK(std::is_same_v<detail::RangeValueT<std::array<int, 3>>, int>);
    STATIC_CHECK(std::is_same_v<detail::RangeValueT<Span<int>>, int>);
    STATIC_CHECK(std::is_same_v<detail::RangeValueT<Span<const int>>, int>);

    STATIC_CHECK(std::is_same_v<detail::RangeValueT<std::vector<bool>>, bool>);
}

TEMPLATE_TEST_CASE("IsRange true", "", (std::vector<int>), (std::array<int, 3>), (Span<int>), (Span<const int>)) {
    STATIC_CHECK(detail::IsRange<TestType>::value);
    STATIC_CHECK(detail::IsRange<TestType&>::value);
    STATIC_CHECK(detail::IsRange<TestType&&>::value);
    STATIC_CHECK(detail::IsRange<const TestType>::value);
    STATIC_CHECK(detail::IsRange<const TestType&>::value);
    STATIC_CHECK(detail::IsRange<const TestType&&>::value);
}

TEMPLATE_TEST_CASE("IsRange false", "", bool, int) {
    STATIC_CHECK_FALSE(detail::IsRange<TestType>::value);
    STATIC_CHECK_FALSE(detail::IsRange<TestType&>::value);
    STATIC_CHECK_FALSE(detail::IsRange<TestType&&>::value);
    STATIC_CHECK_FALSE(detail::IsRange<const TestType>::value);
    STATIC_CHECK_FALSE(detail::IsRange<const TestType&>::value);
    STATIC_CHECK_FALSE(detail::IsRange<const TestType&&>::value);
}

TEMPLATE_TEST_CASE("IsContiguousRange true", "", (std::vector<int>), (std::array<int, 3>), (Span<int>), (Span<const int>)) {
    STATIC_CHECK(detail::IsContiguousRange<TestType>::value);
    STATIC_CHECK(detail::IsContiguousRange<TestType&>::value);
    STATIC_CHECK(detail::IsContiguousRange<TestType&&>::value);
    STATIC_CHECK(detail::IsContiguousRange<const TestType>::value);
    STATIC_CHECK(detail::IsContiguousRange<const TestType&>::value);
    STATIC_CHECK(detail::IsContiguousRange<const TestType&&>::value);
}

TEMPLATE_TEST_CASE("IsContiguousRange false", "", (std::list<int>), (std::vector<bool>)) {
    STATIC_CHECK_FALSE(detail::IsContiguousRange<TestType>::value);
    STATIC_CHECK_FALSE(detail::IsContiguousRange<TestType&>::value);
    STATIC_CHECK_FALSE(detail::IsContiguousRange<TestType&&>::value);
    STATIC_CHECK_FALSE(detail::IsContiguousRange<const TestType>::value);
    STATIC_CHECK_FALSE(detail::IsContiguousRange<const TestType&>::value);
    STATIC_CHECK_FALSE(detail::IsContiguousRange<const TestType&&>::value);
}
