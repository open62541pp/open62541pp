#include <list>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "open62541pp/detail/iterator.hpp"

using namespace opcua;

TEST_CASE("TransformIterator") {
    std::vector<int> vec{1, 2, 3};
    const auto square = [](auto value) { return value * value; };

    auto first = detail::TransformIterator(vec.begin(), square);
    auto last = detail::TransformIterator(vec.end() - 1, square);

    CHECK(std::is_same_v<
          std::iterator_traits<decltype(first)>::iterator_category,
          std::random_access_iterator_tag>);

    SECTION("base") {
        CHECK(first.base() == vec.begin());
        CHECK(detail::TransformIterator(vec.begin(), square).base() == vec.begin());
    }

    SECTION("operator*") {
        CHECK(*first == 1);
        CHECK(*last == 9);
    }

    SECTION("operator[]") {
        CHECK(first[0] == 1);
        CHECK(first[1] == 4);
        CHECK(first[2] == 9);
    }

    SECTION("Increment") {
        auto it = first;
        SECTION("Pre") {
            ++it;
        }
        SECTION("Post") {
            it++;
        }
        SECTION("Add 1") {
            it += 1;
        }
        CHECK(it.base() == (first.base() + 1));
    }

    SECTION("Decrement") {
        auto it = last;
        SECTION("Pre") {
            --it;
        }
        SECTION("Post") {
            it--;
        }
        SECTION("Add 1") {
            it -= 1;
        }
        CHECK(it.base() == (last.base() - 1));
    }

    SECTION("Comparison") {
        CHECK(first == first);
        CHECK_FALSE(first == last);

        CHECK_FALSE(first != first);
        CHECK(first != last);

        CHECK(first < last);
        CHECK_FALSE(first < first);
        CHECK_FALSE(last < first);

        CHECK(first <= last);
        CHECK(first <= first);
        CHECK_FALSE(last <= first);

        CHECK(last > first);
        CHECK_FALSE(last > last);
        CHECK_FALSE(first > last);

        CHECK(last >= first);
        CHECK(last >= last);
        CHECK_FALSE(first >= last);
    }

    SECTION("Distance") {
        CHECK(last - first == 2);
    }
}

TEST_CASE("TransformIterator list -> vector") {
    const std::list<int> lst{1, 2, 3};
    const auto square = [](auto value) { return value * value; };
    const std::vector vec(
        detail::TransformIterator(lst.begin(), square),
        detail::TransformIterator(lst.end(), square)
    );

    CHECK(vec.size() == 3);
    CHECK(vec[0] == 1);
    CHECK(vec[1] == 4);
    CHECK(vec[2] == 9);
}
