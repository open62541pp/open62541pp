#include <array>
#include <list>
#include <vector>

#include "doctest/doctest.h"

#include "open62541pp/detail/traits.hpp"
#include "open62541pp/span.hpp"

using namespace opcua;

TEST_CASE("RangeValueT") {
    CHECK(std::is_same_v<detail::RangeValueT<std::vector<int>>, int>);
    CHECK(std::is_same_v<detail::RangeValueT<std::vector<int>&>, int>);
    CHECK(std::is_same_v<detail::RangeValueT<std::vector<int>&&>, int>);
    CHECK(std::is_same_v<detail::RangeValueT<const std::vector<int>>, int>);
    CHECK(std::is_same_v<detail::RangeValueT<const std::vector<int>&>, int>);
    CHECK(std::is_same_v<detail::RangeValueT<const std::vector<int>&&>, int>);

    CHECK(std::is_same_v<detail::RangeValueT<int[3]>, int>);  // NOLINT(*avoid-c-arrays)
    CHECK(std::is_same_v<detail::RangeValueT<std::array<int, 3>>, int>);
    CHECK(std::is_same_v<detail::RangeValueT<Span<int>>, int>);
    CHECK(std::is_same_v<detail::RangeValueT<Span<const int>>, int>);

    CHECK(std::is_same_v<detail::RangeValueT<std::vector<bool>>, bool>);
}

TEST_CASE_TEMPLATE("IsRange true", T, std::vector<int>, std::array<int, 3>, Span<int>, Span<const int>) {
    CHECK(detail::IsRange<T>::value);
    CHECK(detail::IsRange<T&>::value);
    CHECK(detail::IsRange<T&&>::value);
    CHECK(detail::IsRange<const T>::value);
    CHECK(detail::IsRange<const T&>::value);
    CHECK(detail::IsRange<const T&&>::value);
}

TEST_CASE_TEMPLATE("IsRange false", T, bool, int) {
    CHECK_FALSE(detail::IsRange<T>::value);
    CHECK_FALSE(detail::IsRange<T&>::value);
    CHECK_FALSE(detail::IsRange<T&&>::value);
    CHECK_FALSE(detail::IsRange<const T>::value);
    CHECK_FALSE(detail::IsRange<const T&>::value);
    CHECK_FALSE(detail::IsRange<const T&&>::value);
}

TEST_CASE_TEMPLATE("IsContiguousRange true", T, std::vector<int>, std::array<int, 3>, Span<int>, Span<const int>) {
    CHECK(detail::IsContiguousRange<T>::value);
    CHECK(detail::IsContiguousRange<T&>::value);
    CHECK(detail::IsContiguousRange<T&&>::value);
    CHECK(detail::IsContiguousRange<const T>::value);
    CHECK(detail::IsContiguousRange<const T&>::value);
    CHECK(detail::IsContiguousRange<const T&&>::value);
}

TEST_CASE_TEMPLATE("IsContiguousRange false", T, std::list<int>, std::vector<bool>) {
    CHECK_FALSE(detail::IsContiguousRange<T>::value);
    CHECK_FALSE(detail::IsContiguousRange<T&>::value);
    CHECK_FALSE(detail::IsContiguousRange<T&&>::value);
    CHECK_FALSE(detail::IsContiguousRange<const T>::value);
    CHECK_FALSE(detail::IsContiguousRange<const T&>::value);
    CHECK_FALSE(detail::IsContiguousRange<const T&&>::value);
}
