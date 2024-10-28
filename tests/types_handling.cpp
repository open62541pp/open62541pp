#include <new>  // bad_alloc

#include <doctest/doctest.h>

#include "open62541pp/detail/types_handling.hpp"

using namespace opcua;

TEST_CASE("Types handling") {
    SUBCASE("isPointerFree") {
        CHECK(detail::isPointerFree<bool>);
        CHECK(detail::isPointerFree<int>);
        CHECK(detail::isPointerFree<float>);
        CHECK(detail::isPointerFree<UA_Guid>);
        CHECK_FALSE(detail::isPointerFree<UA_String>);
        CHECK_FALSE(detail::isPointerFree<UA_NodeId>);
    }

    SUBCASE("Allocate / deallocate") {
        auto* ptr = detail::allocate<UA_String>(UA_TYPES[UA_TYPES_STRING]);
        CHECK(ptr != nullptr);
        detail::deallocate(ptr, UA_TYPES[UA_TYPES_STRING]);
    }

    SUBCASE("Allocate as unique_ptr") {
        auto ptr = detail::allocateUniquePtr<UA_String>(UA_TYPES[UA_TYPES_STRING]);
        CHECK(ptr.get() != nullptr);
    }
}

TEST_CASE("Array handling") {
    SUBCASE("Allocate / deallocate") {
        auto* ptr = detail::allocateArray<UA_String>(3, UA_TYPES[UA_TYPES_STRING]);
        CHECK(ptr != nullptr);
        detail::deallocateArray(ptr, 3, UA_TYPES[UA_TYPES_STRING]);

        SUBCASE("Exceed memory") {
            CHECK_THROWS_AS(
                []() {
                    const auto huge = size_t(-1);
                    return detail::allocateArray<UA_String>(huge, UA_TYPES[UA_TYPES_STRING]);
                }(),
                std::bad_alloc
            );
        }
    }

    SUBCASE("Allocate as unique_ptr") {
        auto ptr = detail::allocateArrayUniquePtr<UA_String>(3, UA_TYPES[UA_TYPES_STRING]);
        CHECK(ptr.get() != nullptr);
    }

    SUBCASE("Copy") {
        SUBCASE("Without pointers") {
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

        SUBCASE("With pointers") {
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
