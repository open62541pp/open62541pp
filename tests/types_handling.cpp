#include <cstring>
#include <stdexcept>

#include <doctest/doctest.h>

#include "open62541pp/detail/types_handling.h"

using namespace opcua;

TEST_CASE("isPointerFree") {
    CHECK(detail::isPointerFree<bool>);
    CHECK(detail::isPointerFree<int>);
    CHECK(detail::isPointerFree<float>);
    CHECK(detail::isPointerFree<UA_Guid>);
    CHECK_FALSE(detail::isPointerFree<UA_String>);
    CHECK_FALSE(detail::isPointerFree<UA_NodeId>);
}

TEST_CASE("Allocate / deallocate") {
    auto* ptr = detail::allocate<UA_String>(UA_TYPES[UA_TYPES_STRING]);
    CHECK(ptr != nullptr);
    detail::deallocate(ptr, UA_TYPES[UA_TYPES_STRING]);
}

TEST_CASE("Allocate as unique_ptr") {
    auto ptr = detail::allocateUniquePtr<UA_String>(UA_TYPES[UA_TYPES_STRING]);
    CHECK(ptr.get() != nullptr);
}

TEST_CASE("Allocate / deallocate array") {
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

TEST_CASE("Allocate array as unique_ptr") {
    auto ptr = detail::allocateArrayUniquePtr<UA_String>(3, UA_TYPES[UA_TYPES_STRING]);
    CHECK(ptr.get() != nullptr);
}
