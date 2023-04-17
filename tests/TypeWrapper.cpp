#include <utility>  // move

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "open62541pp/Common.h"
#include "open62541pp/Helper.h"  // detail::toString
#include "open62541pp/TypeWrapper.h"

#include "open62541_impl.h"

using namespace Catch::Matchers;
using namespace opcua;

TEST_CASE("TypeWrapper") {
    SECTION("Constructor empty") {
        REQUIRE_NOTHROW(TypeWrapper<UA_Boolean, UA_TYPES_BOOLEAN>());
    }

    SECTION("Constructor with native type") {
        UA_Boolean value{true};
        TypeWrapper<UA_Boolean, UA_TYPES_BOOLEAN> wrapper(value);
        REQUIRE(*wrapper.handle() == true);
    }

    SECTION("Constructor with native type (implicit)") {
        TypeWrapper<UA_Int32, UA_TYPES_INT32> wrapper = UA_Int32{123};
        REQUIRE(*wrapper.handle() == 123);
    }

    SECTION("Constructor with wrapper type") {
        TypeWrapper<UA_Double, UA_TYPES_DOUBLE> wrapper(11.11);
        REQUIRE(*wrapper.handle() == 11.11);
    }

    SECTION("Copy constructor") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper(UA_STRING_ALLOC("test"));
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapperConstructor(wrapper);

        REQUIRE(wrapperConstructor.handle()->data != wrapper.handle()->data);
        REQUIRE_THAT(detail::toString(*wrapperConstructor.handle()), Equals("test"));
    }

    SECTION("Copy assignment") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper(UA_STRING_ALLOC("test"));
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapperAssignmnet = wrapper;

        REQUIRE(wrapperAssignmnet.handle()->data != wrapper.handle()->data);
        REQUIRE_THAT(detail::toString(*wrapperAssignmnet.handle()), Equals("test"));

        // self assignment
        REQUIRE_NOTHROW(wrapper = wrapper);
    }

    SECTION("Copy assignment with native type") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper(UA_STRING_ALLOC("overwrite"));
        UA_String str = UA_STRING_ALLOC("test");
        wrapper = str;
        REQUIRE_THAT(detail::toString(*wrapper.handle()), Equals("test"));
        UA_String_clear(&str);
    }

    SECTION("Move constructor") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper(UA_STRING_ALLOC("test"));
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapperConstructor(std::move(wrapper));

        REQUIRE(wrapper.handle()->data == nullptr);
        REQUIRE_THAT(detail::toString(*wrapperConstructor.handle()), Equals("test"));
    }

    SECTION("Move assignment") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper(UA_STRING_ALLOC("test"));
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapperAssignmnet = std::move(wrapper);

        REQUIRE(wrapper.handle()->data == nullptr);
        REQUIRE_THAT(detail::toString(*wrapperAssignmnet.handle()), Equals("test"));

        // self assignment
        REQUIRE_NOTHROW(wrapper = std::move(wrapper));
    }

    SECTION("Move assignment with native type") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper(UA_STRING_ALLOC("overwrite"));
        wrapper = UA_STRING_ALLOC("test");
        REQUIRE_THAT(detail::toString(*wrapper.handle()), Equals("test"));
    }

    SECTION("Implicit conversion") {
        TypeWrapper<UA_Float, UA_TYPES_FLOAT> wrapper(11.11f);

        float& value = wrapper;
        REQUIRE(value == 11.11f);

        const float& cvalue = wrapper;
        REQUIRE(cvalue == 11.11f);
    }

    SECTION("Member access") {
        TypeWrapper<UA_NodeId, UA_TYPES_NODEID> wrapper(UA_NODEID_NUMERIC(1, 1000));
        REQUIRE(wrapper->namespaceIndex == 1);
        REQUIRE(wrapper->identifierType == UA_NODEIDTYPE_NUMERIC);
        REQUIRE(wrapper->identifier.numeric == 1000);
    }

    SECTION("Swap") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper1(UA_STRING_ALLOC("test"));
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper2;

        REQUIRE(wrapper1.handle()->data != nullptr);
        REQUIRE(wrapper2.handle()->data == nullptr);
        REQUIRE_THAT(detail::toString(*wrapper1.handle()), Equals("test"));

        wrapper1.swap(wrapper2);
        REQUIRE(wrapper1.handle()->data == nullptr);
        REQUIRE(wrapper2.handle()->data != nullptr);
        REQUIRE_THAT(detail::toString(*wrapper2.handle()), Equals("test"));
    }

    SECTION("Swap with native object") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper;
        UA_String str = UA_STRING_ALLOC("test");

        REQUIRE(wrapper.handle()->data == nullptr);
        REQUIRE(str.data != nullptr);

        wrapper.swap(str);
        REQUIRE(wrapper.handle()->data != nullptr);
        REQUIRE(str.data == nullptr);

        // UA_String_clear not necessary, because data is now owned by wrapper
    }
}

TEMPLATE_TEST_CASE_SIG(
    "TypeWrapper instantiation",
    "",
    ((typename T, uint16_t typeIndex), T, typeIndex),
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
    SECTION("Memory size") {
        STATIC_REQUIRE(sizeof(TypeWrapper<T, typeIndex>) == sizeof(T));
    }

    SECTION("Get type index") {
        STATIC_REQUIRE(TypeWrapper<T, typeIndex>::getTypeIndex() == typeIndex);
    }

    SECTION("Get data type") {
        REQUIRE(TypeWrapper<T, typeIndex>::getDataType() == &UA_TYPES[typeIndex]);
    }
}

TEST_CASE("asWrapper") {
    class Int32Wrapper : public TypeWrapper<int32_t, UA_TYPES_INT32> {
    public:
        void increment() {
            auto& ref = *handle();
            ++ref;
        }

        int32_t get() const {
            return *handle();
        }
    };

    SECTION("non-const") {
        int32_t value = 1;
        Int32Wrapper& wrapper = asWrapper<Int32Wrapper>(value);
        wrapper.increment();
        REQUIRE(value == 2);
        REQUIRE(wrapper.get() == 2);
    }

    SECTION("const") {
        const int32_t value = 1;
        const Int32Wrapper& wrapper = asWrapper<Int32Wrapper>(value);
        REQUIRE(wrapper.get() == 1);
    }
}
