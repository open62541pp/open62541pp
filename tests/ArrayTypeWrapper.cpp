#include <utility>  // move

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "open62541pp/ArrayTypeWrapper.h"
#include "open62541pp/Helper.h"  // detail::toString
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/Types.h"

using namespace Catch::Matchers;
using namespace opcua;

TEST_CASE("ArrayTypeWrapper") {
    SECTION("Constructor") {
        // Empty constructor
        REQUIRE_NOTHROW(ArrayTypeWrapper<UA_Boolean, UA_TYPES_BOOLEAN>(1));

        // Constructor with wrapped type (lvalue)
        {
            UA_Boolean dArray[1] = {true};
            UA_Boolean* pdArray = &dArray[0];
            ArrayTypeWrapper<UA_Boolean, UA_TYPES_BOOLEAN> wrapper(pdArray, 1);
            REQUIRE(wrapper[0] == true);
        }

        // Constructor with wrapped type (rvalue)
        {
            UA_Double* dArray = reinterpret_cast<UA_Double*>(
                UA_Array_new(1, &UA_TYPES[UA_TYPES_DOUBLE])
            );
            dArray[0] = 11.11;
            ArrayTypeWrapper<UA_Double, UA_TYPES_DOUBLE> wrapper(std::move(dArray), 1);
            REQUIRE(wrapper[0] == 11.11);
        }
    }

    SECTION("Copy constructor / assignment") {
        UA_String sArray[] = {UA_STRING_ALLOC("test")};
        UA_String* psArray = &sArray[0];
        ArrayTypeWrapper<UA_String, UA_TYPES_STRING> wrapper(psArray, 1);
        UA_Array_delete(sArray[0].data, sArray[0].length, &UA_TYPES[UA_TYPES_BYTE]);
        ArrayTypeWrapper<UA_String, UA_TYPES_STRING> wrapperConstructor(wrapper);

        REQUIRE(wrapperConstructor.handle()->data != wrapper.handle()->data);
        REQUIRE_THAT(detail::toString(wrapperConstructor[0]), Equals("test"));
    }

    SECTION("Copy assignment") {
        UA_String sArray[] = {UA_STRING_ALLOC("test")};
        UA_String* psArray = &sArray[0];
        ArrayTypeWrapper<UA_String, UA_TYPES_STRING> wrapper(psArray, 1);
        UA_Array_delete(sArray[0].data, sArray[0].length, &UA_TYPES[UA_TYPES_BYTE]);
        ArrayTypeWrapper<UA_String, UA_TYPES_STRING> wrapperAssignmnet = wrapper;

        REQUIRE(wrapperAssignmnet.handle()->data != wrapper.handle()->data);
        REQUIRE_THAT(detail::toString(wrapperAssignmnet[0]), Equals("test"));

        // self assignment
        REQUIRE_NOTHROW(wrapper = wrapper);
    }

    SECTION("Move constructor") {
        UA_String sArray[] = {UA_STRING_ALLOC("test")};
        UA_String* psArray = &sArray[0];
        ArrayTypeWrapper<UA_String, UA_TYPES_STRING> wrapper(psArray, 1);
        UA_Array_delete(sArray[0].data, sArray[0].length, &UA_TYPES[UA_TYPES_BYTE]);
        ArrayTypeWrapper<UA_String, UA_TYPES_STRING> wrapperConstructor(std::move(wrapper));

        REQUIRE(wrapper.handle() == nullptr);
        REQUIRE_THAT(detail::toString(wrapperConstructor[0]), Equals("test"));
    }

    SECTION("Move assignment") {
        UA_String sArray[] = {UA_STRING_ALLOC("test")};
        UA_String* psArray = &sArray[0];
        ArrayTypeWrapper<UA_String, UA_TYPES_STRING> wrapper(psArray, 1);
        UA_Array_delete(sArray[0].data, sArray[0].length, &UA_TYPES[UA_TYPES_BYTE]);
        ArrayTypeWrapper<UA_String, UA_TYPES_STRING> wrapperAssignmnet = std::move(wrapper);

        REQUIRE(wrapper.handle() == nullptr);
        REQUIRE_THAT(detail::toString(wrapperAssignmnet[0]), Equals("test"));

        // self assignment
        REQUIRE_NOTHROW(wrapper = std::move(wrapper));
    }

    SECTION("Implicit conversion") {
        UA_Float fArray[] = {11.11f, 22.22f};
        UA_Float* pfArray = &fArray[0];
        ArrayTypeWrapper<UA_Float, UA_TYPES_FLOAT> wrapper(pfArray, 2);

        float* value = wrapper;
        REQUIRE(value[0] == 11.11f);

        const float* cvalue = wrapper;
        REQUIRE(cvalue[1] == 22.22f);
    }

    SECTION("Member access") {
        UA_NodeId nArray[] = {UA_NODEID_NUMERIC(1, 1000)};
        UA_NodeId* pnArray = &nArray[0];
        ArrayTypeWrapper<UA_NodeId, UA_TYPES_NODEID> wrapper(pnArray, 1);
        REQUIRE(wrapper.at(0).namespaceIndex == 1);
        REQUIRE(wrapper[0].identifierType == UA_NODEIDTYPE_NUMERIC);
        REQUIRE(wrapper[0].identifier.numeric == 1000);
    }

    SECTION("Swap") {
        UA_String sArray[] = {UA_STRING_ALLOC("test")};
        UA_String* psArray = &sArray[0];
        ArrayTypeWrapper<UA_String, UA_TYPES_STRING> wrapper1(psArray, 1);
        UA_Array_delete(sArray[0].data, sArray[0].length, &UA_TYPES[UA_TYPES_BYTE]);
        ArrayTypeWrapper<UA_String, UA_TYPES_STRING> wrapper2;

        REQUIRE(wrapper1.handle() != nullptr);
        REQUIRE(wrapper2.handle() == nullptr);
        REQUIRE_THAT(detail::toString(wrapper1[0]), Equals("test"));

        wrapper1.swap(wrapper2);
        REQUIRE(wrapper1.handle() == nullptr);
        REQUIRE(wrapper2.handle() != nullptr);
        REQUIRE_THAT(detail::toString(wrapper2[0]), Equals("test"));
    }
}

TEST_CASE("StringArray") {
    SECTION("Constructor") {
        REQUIRE_NOTHROW(StringArray({"test", "123"}));
        {
            StringArray wrapper({"test", "123"});
            REQUIRE_THAT(detail::toString(wrapper[0]), Equals("test"));
            REQUIRE_THAT(detail::toString(wrapper[1]), Equals("123"));
            REQUIRE_THAT(wrapper.get()[0], Equals("test"));
            REQUIRE_THAT(wrapper.get()[1], Equals("123"));
        }
    }
}
