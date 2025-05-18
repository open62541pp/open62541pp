#include <algorithm>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "open62541pp/array.hpp"
#include "open62541pp/types.hpp"

using namespace opcua;

TEST_CASE("Array") {
    SECTION("Default") {
        const Array<int> arr;
        CHECK(arr.size() == 0);
        CHECK(arr.data() == nullptr);
    }

    SECTION("From size") {
        const Array<int> arr(10);
        CHECK(arr.size() == 10);
        CHECK(arr.data() != nullptr);
        CHECK(*arr.data() == 0);
    }
    
    SECTION("From size > UA_INT32_MAX") {
        CHECK_THROWS_AS(Array<int>(size_t{UA_INT32_MAX} + 1), std::bad_alloc);
    }

    SECTION("From count and value") {
        const Array<int> arr(3, 11);
        CHECK(arr.size() == 3);
        CHECK(arr.data() != nullptr);
        CHECK(*arr.data() == 11);
        CHECK(*(arr.data() + 1) == 11);
        CHECK(*(arr.data() + 2) == 11);
    }

    SECTION("From initializer list") {
        const Array<int> arr{1, 2, 3};
        CHECK(arr.size() == 3);
        CHECK(arr.data() != nullptr);
        CHECK(*arr.data() == 1);
    }

    SECTION("From empty iterator pair") {
        std::vector<int> vec;
        const Array<int> arr{vec.begin(), vec.end()};
        CHECK(arr.size() == 0);
        CHECK(arr.data() == nullptr);
    }

    SECTION("From iterator pair") {
        std::vector<int> vec{1, 2, 3};
        const Array<int> arr{vec.begin(), vec.end()};
        CHECK(arr.size() == 3);
        CHECK(arr.data() != nullptr);
        CHECK(*arr.data() == 1);
        CHECK(*(arr.data() + 1) == 2);
        CHECK(*(arr.data() + 2) == 3);
    }

    SECTION("From iterator pair (input iterator, single-pass)") {
        std::string src{"abcdefghijklmnopqrstuvwxyz"};
        std::istringstream ss(src);  // allows only single-pass reading
        std::istream_iterator<char> first{ss}, last;
        const Array<char> arr{first, last};
        CHECK(arr.size() == 26);
        CHECK(arr.data() != nullptr);
        CHECK(std::string_view{arr.data(), arr.size()} == src);
    }

    SECTION("Copy constructor") {
        const Array<double> arr(10);
        const Array<double> arrCopy{arr};
        CHECK(arrCopy.size() == 10);
        CHECK(arr.data() != arrCopy.data());
    }

    SECTION("Move constructor") {
        Array<int> arr(10);
        const Array<int> arrMove{std::move(arr)};
        CHECK(arr.size() == 0);
        CHECK(arr.data() == nullptr);
        CHECK(arrMove.size() == 10);
        CHECK(arrMove.data() != nullptr);
    }

    SECTION("operator[]") {
        Array<int> arr(2);
        arr[1] = 1;
        CHECK(arr[0] == 0);
        CHECK(arr[1] == 1);
        CHECK(std::as_const(arr[1]) == 1);
    }

    SECTION("at") {
        Array<int> arr(2);
        arr.at(1) = 1;
        CHECK(arr.at(0) == 0);
        CHECK(arr.at(1) == 1);
        CHECK(std::as_const(arr).at(1) == 1);
        CHECK_THROWS(arr.at(2));
        CHECK_THROWS(std::as_const(arr).at(2));
    }

    SECTION("front") {
        Array<int> arr{1, 2};
        CHECK(arr.front() == 1);
        arr.front() = 10;
        CHECK(arr.front() == 10);
        CHECK(std::as_const(arr).front() == 10);
    }

    SECTION("back") {
        Array<int> arr{1, 2};
        CHECK(arr.back() == 2);
        arr.back() = 20;
        CHECK(arr.back() == 20);
        CHECK(std::as_const(arr).back() == 20);
    }

    SECTION("clear") {
        Array<String> arr{String{"a"}, String{"b"}, String{"c"}};
        arr.clear();
        CHECK(arr.size() == 0);
        CHECK(arr.data() == nullptr);
    }

    SECTION("swap") {
        Array<int> arr{1, 2, 3};
        Array<int> arrSwap;
        arr.swap(arrSwap);
        CHECK(arr.size() == 0);
        CHECK(arr.data() == nullptr);
        CHECK(arrSwap.size() == 3);
        CHECK(arrSwap.data() != nullptr);
    }

    SECTION("release") {
        Array<int> arr{1, 2, 3};
        int* ptr = arr.release();
        CHECK(ptr != nullptr);
        CHECK(arr.size() == 0);
        CHECK(arr.data() == nullptr);
        UA_free(ptr);
    }
}

TEMPLATE_TEST_CASE("Array iterators", "", Array<char>, const Array<char>) {
    TestType arr{'a', 'b', 'c'};

    SECTION("begin(), end() iterators") {
        CHECK(*arr.begin() == 'a');
        CHECK(*(arr.begin() + 1) == 'b');
        CHECK(*(arr.begin() + 2) == 'c');
        CHECK(std::distance(arr.begin(), arr.end()) == 3);
        CHECK(std::distance(arr.cbegin(), arr.cend()) == 3);

        std::string result;
        std::for_each(arr.begin(), arr.end(), [&](char c) { result += c; });
        CHECK(result == "abc");
    }

    SECTION("rbegin(), rend() iterators") {
        CHECK(*arr.rbegin() == 'c');
        CHECK(*(arr.rbegin() + 1) == 'b');
        CHECK(*(arr.rbegin() + 2) == 'a');
        CHECK(std::distance(arr.rbegin(), arr.rend()) == 3);
        CHECK(std::distance(arr.crbegin(), arr.crend()) == 3);

        std::string result;
        std::for_each(arr.rbegin(), arr.rend(), [&](char c) { result += c; });
        CHECK(result == "cba");
    }
}

TEMPLATE_TEST_CASE("Array resize", "", String, std::string, std::string_view) {
    Array<TestType> arr{TestType{"a"}, TestType{"b"}, TestType{"c"}};
    const auto* ptr = arr.data();

    SECTION("newSize == size") {
        arr.resize(3);
        CHECK(arr.size() == 3);
        CHECK(arr.data() == ptr);
    }

    SECTION("newSize > size") {
        arr.resize(5);
        CHECK(arr.size() == 5);
        CHECK(arr.data() != nullptr);
        CHECK(arr.data() != ptr);
        CHECK(arr.at(0) == "a");
        CHECK(arr.at(1) == "b");
        CHECK(arr.at(2) == "c");
        CHECK(arr.at(3).empty());
        CHECK(arr.at(4).empty());
    }

    SECTION("newSize < size") {
        arr.resize(2);
        CHECK(arr.size() == 2);
        CHECK(arr.data() != nullptr);
        CHECK(arr.data() != ptr);
        CHECK(arr.at(0) == "a");
        CHECK(arr.at(1) == "b");
    }

    SECTION("newSize == 0") {
        arr.resize(0);
        CHECK(arr.size() == 0);
        CHECK(arr.data() == nullptr);
    }

    SECTION("newSize > limit") {
        CHECK_THROWS_AS(arr.resize(static_cast<size_t>(-1)), std::bad_alloc);
    }
}

TEST_CASE("Array assign") {
    Array arr{1, 2, 3};

    SECTION("assign count and value") {
        arr.assign(3, 1);
        CHECK(arr == Array{1, 1, 1});
    }

    SECTION("assign range") {
        std::vector<int> range{4, 5, 6};
        arr.assign(range.begin(), range.end());
        CHECK(arr == Array{4, 5, 6});
    }

    SECTION("assign initializer list") {
        arr.assign({4, 5, 6});
        CHECK(arr == Array{4, 5, 6});
    }
}

template <typename C>
static std::string concat(const C& arr) {
    std::string str;
    std::for_each(arr.begin(), arr.end(), [&](auto& e) { str.append(e); });
    return str;
}

TEMPLATE_TEST_CASE("Array insert", "", (Array<std::string>), (std::vector<std::string>)) {
    TestType arr{"a", "b", "c"};
    CHECK(concat(arr) == "abc");

    SECTION("prepend value") {
        const auto it = arr.insert(arr.begin(), "x");
        CHECK(concat(arr) == "xabc");
        CHECK(it == arr.begin());
    }
    
    SECTION("append value") {
        SECTION("lvalue") {
            const std::string value = "lvalue";
            const auto it = arr.insert(arr.end(), value);
            CHECK(arr.size() == 4);
            CHECK(arr.back() == "lvalue");
            CHECK(it == arr.end() - 1);
        }
        SECTION("rvalue") {
            const auto it = arr.insert(arr.end(), "rvalue");
            CHECK(arr.size() == 4);
            CHECK(arr.back() == "rvalue");
            CHECK(it == arr.end() - 1);
        }
    }
    
    SECTION("insert value") {
        const auto it = arr.insert(arr.begin() + 1, "x");
        CHECK(concat(arr) == "axbc");
        CHECK(it == arr.begin() + 1);
    }
    
    SECTION("insert count of values") {
        const auto it = arr.insert(arr.begin() + 1, 3, "x");
        CHECK(concat(arr) == "axxxbc");
        CHECK(it == arr.begin() + 1);
    }

    SECTION("insert range") {
        const TestType range{"x", "y", "z"};
        const auto it = arr.insert(arr.begin() + 2, range.begin(), range.end());
        CHECK(concat(arr) == "abxyzc");
        CHECK(it == arr.begin() + 2);
    }

    SECTION("insert empty range") {
        const TestType range{};
        const auto it1 = arr.begin();
        const auto it2 = arr.insert(arr.begin(), range.begin(), range.end());
        CHECK(concat(arr) == "abc");
        CHECK(it1 == it2);
    }
}
    
TEST_CASE("Array insert (input iterator, single pass)") {
    std::istringstream ss{"bc"};  // allows only single-pass reading
    std::istream_iterator<char> first{ss}, last;
    Array<char> arr{'a'};
    arr.insert(arr.end(), first, last);
    CHECK(arr.size() == 3);
    CHECK(arr[0] == 'a');
    CHECK(arr[1] == 'b');
    CHECK(arr[2] == 'c');

}

TEMPLATE_TEST_CASE("Array erase", "", (Array<std::string>), (std::vector<std::string>)) {
    TestType arr{"a", "b", "c", "d", "e"};
    CHECK(concat(arr) == "abcde");

    SECTION("erase first") {
        const auto it = arr.erase(arr.begin());
        CHECK(concat(arr) == "bcde");
        CHECK(it == arr.begin());
    }
    
    SECTION("erase value") {
        const auto it = arr.erase(arr.begin() + 1);
        CHECK(concat(arr) == "acde");
        CHECK(it == arr.begin() + 1);
    }

    SECTION("erase last") {
        const auto it = arr.erase(arr.end() - 1);
        CHECK(concat(arr) == "abcd");
        CHECK(it == arr.end());
    }

    SECTION("erase empty range") {
        const auto it = arr.erase(arr.begin(), arr.begin());
        CHECK(concat(arr) == "abcde");
        CHECK(it == arr.begin());
    }

    SECTION("erase range") {
        const auto it = arr.erase(arr.begin() + 1, arr.begin() + 3);
        CHECK(concat(arr) == "ade");
        CHECK(it == arr.begin() + 1);
    }
}
