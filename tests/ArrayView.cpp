#include <array>
#include <vector>

#include <doctest/doctest.h>

#include "open62541pp/ArrayView.h"

using namespace opcua;

TEST_CASE("ArrayView") {
    SUBCASE("Empty") {
        constexpr ArrayView<int> view;
        CHECK(view.size() == 0);
        CHECK(view.empty());
        CHECK(view.data() == nullptr);
    }

    SUBCASE("From array") {
        constexpr std::array<int, 3> arr{0, 1, 2};
        ArrayView view(arr);
        CHECK(view.size() == arr.size());
        CHECK(!view.empty());
        CHECK(view.data() == arr.data());
    }

    SUBCASE("From vector") {
        std::vector<int> vec{0, 1, 2};
        ArrayView view(vec);
        CHECK(view.size() == vec.size());
        CHECK(!view.empty());
        CHECK(view.data() == vec.data());
    }

    SUBCASE("Element access") {
        std::vector<int> vec{0, 1, 2};
        ArrayView view(vec);

        CHECK(view.front() == 0);
        CHECK(view.back() == 2);
        CHECK(view.data() == vec.data());

        CHECK(view[2] == 2);
        view[2] = 20;
        CHECK(view[2] == 20);
        CHECK(vec[2] == 20);
    }

    SUBCASE("Iterators") {
        const std::vector<int> vec{0, 1, 2};
        ArrayView view(vec);

        CHECK(std::vector<int>(view.begin(), view.end()) == std::vector{0, 1, 2});
        CHECK(std::vector<int>(view.cbegin(), view.cend()) == std::vector{0, 1, 2});
        CHECK(std::vector<int>(view.rbegin(), view.rend()) == std::vector{2, 1, 0});
        CHECK(std::vector<int>(view.crbegin(), view.crend()) == std::vector{2, 1, 0});
    }

    SUBCASE("Subviews") {
        const std::vector<int> vec{0, 1, 2};
        ArrayView view(vec);

        CHECK(view.subview(0).size() == 3);
        CHECK(view.subview(0).data() == view.begin());

        CHECK(view.subview(1).size() == 2);
        CHECK(view.subview(1).data() == view.begin() + 1);

        CHECK(view.subview(1, 1).size() == 1);
        CHECK(view.subview(1, 1).data() == view.begin() + 1);

        CHECK(view.subview(1, 10).size() == 2);
        CHECK(view.subview(1, 10).data() == view.begin() + 1);

        CHECK(view.subview(10).size() == 0);
        CHECK(view.subview(10).data() == nullptr);

        CHECK(view.first(2).size() == 2);
        CHECK(view.first(2).data() == view.begin());

        CHECK(view.first(10).size() == 3);
        CHECK(view.first(10).data() == view.begin());

        CHECK(view.last(1).size() == 1);
        CHECK(view.last(1).data() == view.begin() + 2);

        CHECK(view.last(10).size() == 3);
        CHECK(view.last(10).data() == view.begin());
    }
}

TEST_CASE("ArrayView as generic function parameter") {
    auto getSize = [](ArrayView<const int> view) { return view.size(); };

    const std::vector vec{0, 1, 2};
    const std::array arr{0, 1, 2};

    CHECK(getSize({vec.data(), vec.size()}) == 3);

    CHECK(getSize(vec) == 3);
    CHECK(getSize(arr) == 3);

    CHECK(getSize(std::vector{0, 1, 2}) == 3);
    CHECK(getSize(std::array{0, 1, 2}) == 3);

    CHECK(getSize({0, 1, 2}) == 3);
}
