#include <new>  // bad_alloc
#include <sstream>

#include <catch2/catch_test_macros.hpp>

#include "open62541pp/detail/types_handling.hpp"

using namespace opcua;

TEST_CASE("Types handling") {
    SECTION("IsPointerFree") {
        CHECK(detail::IsPointerFree<bool>::value);
        CHECK(detail::IsPointerFree<int>::value);
        CHECK(detail::IsPointerFree<float>::value);
        CHECK(detail::IsPointerFree<UA_Guid>::value);
        CHECK_FALSE(detail::IsPointerFree<UA_String>::value);
        CHECK_FALSE(detail::IsPointerFree<UA_NodeId>::value);
    }

    SECTION("Allocate / deallocate") {
        auto* ptr = detail::allocate<UA_String>();
        CHECK(ptr != nullptr);
        detail::deallocate(ptr, UA_TYPES[UA_TYPES_STRING]);
    }

    SECTION("Allocate as unique_ptr") {
        auto ptr = detail::makeUnique<UA_String>(UA_TYPES[UA_TYPES_STRING]);
        CHECK(ptr.get() != nullptr);
    }
}

TEST_CASE("Array handling") {
    SECTION("Allocate empty array") {
        auto* ptr = detail::allocateArray<int>(0);
        CHECK(ptr != nullptr);
        CHECK(ptr == UA_EMPTY_ARRAY_SENTINEL);  // NOLINT
        detail::deallocateArray(ptr);
    }

    SECTION("Allocate / deallocate") {
        auto* ptr = detail::allocateArray<UA_String>(3);
        CHECK(ptr != nullptr);
        detail::deallocateArray(ptr, 3, UA_TYPES[UA_TYPES_STRING]);

        SECTION("Exceed memory") {
            CHECK_THROWS_AS(
                []() {
                    const auto huge = size_t(-1);
                    return detail::allocateArray<UA_String>(huge);
                }(),
                std::bad_alloc
            );
        }
    }

    SECTION("Copy") {
        SECTION("Empty array") {
            UA_Int32* src = nullptr;
            CHECK(detail::copyArray(src, 0) == UA_EMPTY_ARRAY_SENTINEL);
            CHECK(detail::copyArray(src, 0, UA_TYPES[UA_TYPES_INT32]) == UA_EMPTY_ARRAY_SENTINEL);

            const auto [ptr, size] = detail::copyArray(src, src, UA_TYPES[UA_TYPES_INT32]);
            CHECK(size == 0);
            CHECK(ptr == UA_EMPTY_ARRAY_SENTINEL);
        }

        SECTION("From pointer (pointer-free)") {
            const std::vector<UA_Int32> src{1, 2};
            const auto& type = UA_TYPES[UA_TYPES_INT32];
            auto* dst = detail::copyArray(src.data(), src.size(), type);
            CHECK(dst[0] == src[0]);
            CHECK(dst[1] == src[1]);
            detail::deallocateArray(dst);
        }

        SECTION("From pointer") {
            const std::vector<UA_String> src{UA_STRING_STATIC("one"), UA_STRING_STATIC("two")};
            const auto& type = UA_TYPES[UA_TYPES_STRING];
            auto* dst = detail::copyArray(src.data(), src.size(), type);
            CHECK(UA_String_equal(&dst[0], &src[0]));
            CHECK(UA_String_equal(&dst[1], &src[1]));
            detail::deallocateArray(dst, src.size(), type);
        }

        SECTION("From iterator pair") {
            const std::vector<UA_String> src{UA_STRING_STATIC("one"), UA_STRING_STATIC("two")};
            const auto& type = UA_TYPES[UA_TYPES_STRING];
            auto [dst, size] = detail::copyArray(src.begin(), src.end(), type);
            CHECK(size == src.size());
            CHECK(UA_String_equal(&dst[0], &src[0]));
            CHECK(UA_String_equal(&dst[1], &src[1]));
            detail::deallocateArray(dst, src.size(), type);
        }

        SECTION("From iterator pair (input iterator, single-pass)") {
            std::istringstream ss{"abcdefghijklmnopqrstuvwxyz"};  // allows only single-pass reading
            std::istream_iterator<char> first(ss), last;
            const auto& type = UA_TYPES[UA_TYPES_BYTE];
            auto [dst, size] = detail::copyArray(first, last, type);
            CHECK(size == 26);
            CHECK(dst[0] == 'a');
            CHECK(dst[1] == 'b');
            CHECK(dst[2] == 'c');
            CHECK(dst[25] == 'z');
            detail::deallocateArray(dst, size, type);
        }
    }
}
