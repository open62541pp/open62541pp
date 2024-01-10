#include <cstring>
#include <stdexcept>

#include <doctest/doctest.h>

#include "open62541_impl.h"  // UA_String_clear
#include "open62541pp/detail/helper.h"

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

TEST_CASE("getDataType") {
    const auto* expected = &UA_TYPES[UA_TYPES_BOOLEAN];
    CHECK(&detail::getDataType(UA_TYPES_BOOLEAN) == expected);
    CHECK(&detail::getDataType(Type::Boolean) == expected);
    CHECK(&detail::getDataType<UA_TYPES_BOOLEAN>() == expected);
    CHECK(&detail::getDataType<Type::Boolean>() == expected);
}

TEST_CASE("UA_String from string_view") {
    SUBCASE("Test string") {
        std::string_view sv("test123");
        UA_String str = detail::toNativeString(sv);
        CHECK(str.length == sv.size());
        CHECK((void*)str.data == (void*)sv.data());
    }

    SUBCASE("Null string") {
        UA_String str = detail::toNativeString({});
        CHECK(str.length == 0);
        CHECK(str.data == nullptr);
    }

    SUBCASE("Empty string") {
        UA_String str = detail::toNativeString("");
        CHECK(str.length == 0);
        CHECK(str.data != nullptr);
    }
}

TEST_CASE("Alloc UA_String from string_view") {
    const char* cstr = "test123";
    std::string_view sv(cstr);
    auto str = detail::allocNativeString(sv);
    CHECK(str.length == 7);
    CHECK(std::strncmp((char*)str.data, sv.data(), 7) == 0);  // NOLINT
    UA_String_clear(&str);
}

TEST_CASE("Alloc UA_String from non-null-terminated string_view") {
    std::string str("test123");
    std::string_view sv(str.c_str(), 4);
    auto uaString = detail::allocNativeString(sv);
    CHECK(uaString.length == 4);
    CHECK(std::strncmp((char*)uaString.data, sv.data(), 4) == 0);  // NOLINT
    UA_String_clear(&uaString);
}

TEST_CASE("String conversion UA_String -> string") {
    SUBCASE("Test string") {
        UA_String str = UA_STRING_ALLOC("test123");
        CHECK(detail::toString(str) == "test123");
        UA_String_clear(&str);
    }

    SUBCASE("Null string") {
        UA_String str{};
        CHECK(detail::toString(str) == "");
    }

    SUBCASE("Empty string") {
        UA_String str = UA_STRING_ALLOC("");
        CHECK(detail::toString(str) == "");
        UA_String_clear(&str);
    }
}

TEST_CASE("ScopeExit") {
    SUBCASE("Common") {
        bool executed = false;
        {
            auto exit = detail::ScopeExit([&] { executed = true; });
        }
        CHECK(executed);
    }

    SUBCASE("Move constructor") {
        int executions = 0;
        {
            detail::ScopeExit exit1([&] { executions++; });
            detail::ScopeExit exit2(std::move(exit1));
        }
        CHECK(executions == 1);
    }

    SUBCASE("Release") {
        bool executed = false;
        {
            auto exit = detail::ScopeExit([&] { executed = true; });
            exit.release();
        }
        CHECK_FALSE(executed);
    }
}
