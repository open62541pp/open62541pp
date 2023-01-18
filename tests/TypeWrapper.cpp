#include <utility>  // move

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "open62541pp/Helper.h"  // detail::toString
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/Types.h"

using namespace Catch::Matchers;
using namespace opcua;

TEST_CASE("TypeWrapper") {
    SECTION("Constructor") {
        // Empty constructor
        REQUIRE_NOTHROW(TypeWrapper<bool, Type::Boolean>());

        // Constructor with wrapped type (lvalue)
        {
            bool value{true};
            TypeWrapper<bool, Type::Boolean> wrapper(value);
            REQUIRE(*wrapper.handle() == value);
        }

        // Constructor with wrapped type (rvalue)
        {
            TypeWrapper<double, Type::Double> wrapper(11.11);
            REQUIRE(*wrapper.handle() == 11.11);
        }
    }

    SECTION("Copy constructor / assignment") {
        TypeWrapper<UA_String, Type::String> wrapper(UA_STRING_ALLOC("test"));
        TypeWrapper<UA_String, Type::String> wrapperConstructor(wrapper);

        REQUIRE(wrapperConstructor.handle()->data != wrapper.handle()->data);
        REQUIRE_THAT(detail::toString(*wrapperConstructor.handle()), Equals("test"));
    }

    SECTION("Copy assignment") {
        TypeWrapper<UA_String, Type::String> wrapper(UA_STRING_ALLOC("test"));
        TypeWrapper<UA_String, Type::String> wrapperAssignmnet = wrapper;

        REQUIRE(wrapperAssignmnet.handle()->data != wrapper.handle()->data);
        REQUIRE_THAT(detail::toString(*wrapperAssignmnet.handle()), Equals("test"));

        // self assignment
        REQUIRE_NOTHROW(wrapper = wrapper);
    }

    SECTION("Move constructor") {
        TypeWrapper<UA_String, Type::String> wrapper(UA_STRING_ALLOC("test"));
        TypeWrapper<UA_String, Type::String> wrapperConstructor(std::move(wrapper));

        REQUIRE(wrapper.handle()->data == nullptr);
        REQUIRE_THAT(detail::toString(*wrapperConstructor.handle()), Equals("test"));
    }

    SECTION("Move assignment") {
        TypeWrapper<UA_String, Type::String> wrapper(UA_STRING_ALLOC("test"));
        TypeWrapper<UA_String, Type::String> wrapperAssignmnet = std::move(wrapper);

        REQUIRE(wrapper.handle()->data == nullptr);
        REQUIRE_THAT(detail::toString(*wrapperAssignmnet.handle()), Equals("test"));

        // self assignment
        REQUIRE_NOTHROW(wrapper = std::move(wrapper));
    }

    SECTION("Swap") {
        TypeWrapper<UA_String, Type::String> wrapper1(UA_STRING_ALLOC("test"));
        TypeWrapper<UA_String, Type::String> wrapper2;

        REQUIRE(wrapper1.handle()->data != nullptr);
        REQUIRE(wrapper2.handle()->data == nullptr);
        REQUIRE_THAT(detail::toString(*wrapper1.handle()), Equals("test"));

        wrapper1.swap(wrapper2);
        REQUIRE(wrapper1.handle()->data == nullptr);
        REQUIRE(wrapper2.handle()->data != nullptr);
        REQUIRE_THAT(detail::toString(*wrapper2.handle()), Equals("test"));
    }
}

TEMPLATE_TEST_CASE_SIG(
    "TypeWrapper instantiation",
    "",
    ((typename T, int typeIndex), T, typeIndex),
    (UA_Boolean, UA_TYPES_BOOLEAN),
    (UA_SByte, UA_TYPES_SBYTE),
    (UA_Byte, UA_TYPES_BYTE),
    (UA_Int16, UA_TYPES_INT16),
    (UA_UInt16, UA_TYPES_UINT16),
    (UA_Int32, UA_TYPES_INT32),
    (UA_UInt32, UA_TYPES_UINT32),
    (UA_Int64, UA_TYPES_INT64),
    (UA_UInt64, UA_TYPES_UINT64),
    (UA_Float, UA_TYPES_FLOAT),
    (UA_Double, UA_TYPES_DOUBLE),
    (UA_String, UA_TYPES_STRING),
    (UA_DateTime, UA_TYPES_DATETIME),
    (UA_Guid, UA_TYPES_GUID),
    (UA_ByteString, UA_TYPES_BYTESTRING),
    (UA_XmlElement, UA_TYPES_XMLELEMENT),
    (UA_NodeId, UA_TYPES_NODEID),
    (UA_ExpandedNodeId, UA_TYPES_EXPANDEDNODEID),
    (UA_StatusCode, UA_TYPES_STATUSCODE),
    (UA_QualifiedName, UA_TYPES_QUALIFIEDNAME),
    (UA_LocalizedText, UA_TYPES_LOCALIZEDTEXT),
    (UA_ExtensionObject, UA_TYPES_EXTENSIONOBJECT),
    (UA_DataValue, UA_TYPES_DATAVALUE),
    (UA_Variant, UA_TYPES_VARIANT),
    (UA_DiagnosticInfo, UA_TYPES_DIAGNOSTICINFO)
) {
    constexpr auto type = static_cast<Type>(typeIndex);

    SECTION("Swap") {
        TypeWrapper<T, type> wrapper1;
        TypeWrapper<T, type> wrapper2;

        wrapper1.swap(wrapper2);
    }

    SECTION("Get type") {
        STATIC_REQUIRE(TypeWrapper<T, type>::getType() == type);
    }

    SECTION("Get data type") {
        REQUIRE(TypeWrapper<T, type>::getDataType() == &UA_TYPES[typeIndex]);
    }
}

TEMPLATE_TEST_CASE("StringLike", "", String, ByteString, XmlElement) {
    SECTION("Construct with const char*") {
        TestType wrapper("test");
        REQUIRE(wrapper.handle()->length == 4);
        REQUIRE_THAT(wrapper.get(), Equals("test"));
        REQUIRE_THAT(std::string(wrapper.getView()), Equals("test"));
    }

    SECTION("Construct from non-null-terminated view") {
        std::string str("test123");
        std::string_view sv(str.c_str(), 4);
        TestType wrapper(sv);
        REQUIRE_THAT(wrapper.get(), Equals("test"));
    }

    SECTION("Equality") {
        REQUIRE(TestType("test") == TestType("test"));
        REQUIRE(TestType("test") != TestType());
    }
}

TEST_CASE("Guid") {
    UA_UInt32 data1{11};
    UA_UInt16 data2{22};
    UA_UInt16 data3{33};
    std::array<UA_Byte, 8> data4{1, 2, 3, 4, 5, 6, 7, 8};

    Guid wrapper(data1, data2, data3, data4);

    REQUIRE(wrapper.handle()->data1 == data1);
    REQUIRE(wrapper.handle()->data2 == data2);
    REQUIRE(wrapper.handle()->data3 == data3);
    for (int i = 0; i < 8; ++i) {
        REQUIRE(wrapper.handle()->data4[i] == data4[i]);  // NOLINT
    }
}
