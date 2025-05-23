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

    SECTION("Allocate as unique_ptr") {
        auto ptr = detail::makeUniqueArray<UA_String>(3, UA_TYPES[UA_TYPES_STRING]);
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
            UA_Int32 src[2] = {1, 2};

            SECTION("From pointer") {
                auto* dst = detail::copyArray(src, size, type);
                CHECK(dst[0] == 1);
                CHECK(dst[1] == 2);
                detail::deallocateArray(dst, size, type);
            }
        }

        SECTION("With pointers") {
            const size_t size = 2;
            const auto& type = UA_TYPES[UA_TYPES_STRING];
            UA_String src[2] = {
                UA_STRING_STATIC("one"),
                UA_STRING_STATIC("two"),
            };

            SECTION("From pointer") {
                auto* dst = detail::copyArray(src, size, type);
                CHECK(UA_String_equal(&dst[0], &src[0]));
                CHECK(UA_String_equal(&dst[1], &src[1]));
                detail::deallocateArray(dst, size, type);
            }
        }
    }
}
