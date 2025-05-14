#include <new>  // bad_alloc

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
        auto* ptr = detail::allocate<UA_String>(UA_TYPES[UA_TYPES_STRING]);
        CHECK(ptr != nullptr);
        detail::deallocate(ptr, UA_TYPES[UA_TYPES_STRING]);
    }

    SECTION("Allocate as unique_ptr") {
        auto ptr = detail::allocateUniquePtr<UA_String>(UA_TYPES[UA_TYPES_STRING]);
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
        auto* ptr = detail::allocateArray<UA_String>(3, UA_TYPES[UA_TYPES_STRING]);
        CHECK(ptr != nullptr);
        detail::deallocateArray(ptr, 3, UA_TYPES[UA_TYPES_STRING]);

        SECTION("Exceed memory") {
            CHECK_THROWS_AS(
                []() {
                    const auto huge = size_t(-1);
                    return detail::allocateArray<UA_String>(huge, UA_TYPES[UA_TYPES_STRING]);
                }(),
                std::bad_alloc
            );
        }
    }

    SECTION("Allocate as unique_ptr") {
        auto ptr = detail::allocateArrayUniquePtr<UA_String>(3, UA_TYPES[UA_TYPES_STRING]);
        CHECK(ptr.get() != nullptr);
    }

    SECTION("Copy") {
        SECTION("Empty array") {
            UA_Int32* src = nullptr;
            CHECK(detail::copyArray(src, 0) == UA_EMPTY_ARRAY_SENTINEL);
            CHECK(detail::copyArray(src, 0, UA_TYPES[UA_TYPES_INT32]) == UA_EMPTY_ARRAY_SENTINEL);
        }

        SECTION("Without pointers") {
            const size_t size = 2;
            const auto& type = UA_TYPES[UA_TYPES_INT32];
            auto* src = detail::allocateArray<UA_Int32>(size, type);
            src[0] = 1;
            src[1] = 2;

            auto* dst = detail::copyArray(src, size, type);
            CHECK(dst[0] == 1);
            CHECK(dst[1] == 2);

            detail::deallocateArray(src, size, type);
            detail::deallocateArray(dst, size, type);
        }

        SECTION("With pointers") {
            const size_t size = 2;
            const auto& type = UA_TYPES[UA_TYPES_STRING];
            auto* src = detail::allocateArray<UA_String>(size, type);
            src[0] = UA_STRING_ALLOC("one");
            src[1] = UA_STRING_ALLOC("two");

            auto* dst = detail::copyArray(src, size, type);
            CHECK(UA_String_equal(&dst[0], &src[0]));
            CHECK(UA_String_equal(&dst[1], &src[1]));

            detail::deallocateArray(src, size, type);
            detail::deallocateArray(dst, size, type);
        }
    }
}
