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
        REQUIRE_NOTHROW(TypeWrapper<UA_Boolean, UA_TYPES_BOOLEAN>());

        // Constructor with wrapped type (lvalue)
        {
            UA_Boolean value{true};
            TypeWrapper<UA_Boolean, UA_TYPES_BOOLEAN> wrapper(value);
            REQUIRE(*wrapper.handle() == value);
        }

        // Constructor with wrapped type (rvalue)
        {
            TypeWrapper<UA_Double, UA_TYPES_DOUBLE> wrapper(11.11);
            REQUIRE(*wrapper.handle() == 11.11);
        }
    }

    SECTION("Copy constructor / assignment") {
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
    SECTION("Swap") {
        TypeWrapper<T, typeIndex> wrapper1;
        TypeWrapper<T, typeIndex> wrapper2;

        wrapper1.swap(wrapper2);
    }

    SECTION("Get type") {
        STATIC_REQUIRE(TypeWrapper<T, typeIndex>::getTypeIndex() == typeIndex);
    }

    SECTION("Get data type") {
        REQUIRE(TypeWrapper<T, typeIndex>::getDataType() == &UA_TYPES[typeIndex]);
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

TEST_CASE("DateTime") {
    SECTION("Empty") {
        const DateTime dt;
        REQUIRE(dt.get() == 0);
        REQUIRE(*dt.handle() == 0);
        // UA time starts before Unix time -> 0
        REQUIRE(dt.toTimePoint() == std::chrono::system_clock::time_point{});
        REQUIRE(dt.toUnixTime() == 0);

        const auto dts = dt.toStruct();
        REQUIRE(dts.nanoSec == 0);
        REQUIRE(dts.microSec == 0);
        REQUIRE(dts.milliSec == 0);
        REQUIRE(dts.sec == 0);
        REQUIRE(dts.min == 0);
        REQUIRE(dts.hour == 0);
        REQUIRE(dts.day == 1);
        REQUIRE(dts.month == 1);
        REQUIRE(dts.year == 1601);
    }

    SECTION("From std::chrono::time_point") {
        using namespace std::chrono;

        const auto now = system_clock::now();
        const uint64_t secSinceEpoch = duration_cast<seconds>(now.time_since_epoch()).count();
        const uint64_t nsecSinceEpoch = duration_cast<nanoseconds>(now.time_since_epoch()).count();

        const DateTime dt(now);
        REQUIRE(dt.get() == (nsecSinceEpoch / 100) + UA_DATETIME_UNIX_EPOCH);
        REQUIRE(dt.toUnixTime() == secSinceEpoch);
    }

    SECTION("Comparison") {
        const auto zero = DateTime(0);
        const auto now = DateTime::now();
        REQUIRE(zero != now);
        REQUIRE(zero < now);
    }
}
