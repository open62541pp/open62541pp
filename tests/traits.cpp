#include <array>
#include <list>
#include <vector>

#include "doctest/doctest.h"

#include "open62541pp/Span.h"
#include "open62541pp/detail/traits.h"

using namespace opcua;

TEST_CASE_TEMPLATE("IsContiguousContainer true", T, std::vector<int>, std::array<int, 3>, Span<int>, Span<const int>) {
    CHECK(detail::IsContiguousContainer<T>::value);
    CHECK(detail::IsContiguousContainer<T&>::value);
    CHECK(detail::IsContiguousContainer<T&&>::value);
    CHECK(detail::IsContiguousContainer<const T>::value);
    CHECK(detail::IsContiguousContainer<const T&>::value);
    CHECK(detail::IsContiguousContainer<const T&&>::value);
}

TEST_CASE_TEMPLATE("IsContiguousContainer false", T, std::list<int>, std::vector<bool>) {
    CHECK_FALSE(detail::IsContiguousContainer<T>::value);
    CHECK_FALSE(detail::IsContiguousContainer<T&>::value);
    CHECK_FALSE(detail::IsContiguousContainer<T&&>::value);
    CHECK_FALSE(detail::IsContiguousContainer<const T>::value);
    CHECK_FALSE(detail::IsContiguousContainer<const T&>::value);
    CHECK_FALSE(detail::IsContiguousContainer<const T&&>::value);
}

TEST_CASE_TEMPLATE("IsMutableContainer true", T, std::vector<int>, std::array<int, 3>, Span<int>) {
    CHECK(detail::IsMutableContainer<T>::value);
    CHECK(detail::IsMutableContainer<T&>::value);
    CHECK(detail::IsMutableContainer<T&&>::value);
}

TEST_CASE_TEMPLATE("IsMutableContainer false", T, const std::vector<int>, const std::array<int, 3>, const Span<const int>) {
    CHECK_FALSE(detail::IsMutableContainer<T>::value);
    CHECK_FALSE(detail::IsMutableContainer<T&>::value);
    CHECK_FALSE(detail::IsMutableContainer<T&&>::value);
}
