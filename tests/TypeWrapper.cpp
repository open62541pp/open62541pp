#include <utility>  // move

#include <doctest/doctest.h>

#include "open62541pp/Common.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/helper.h"  // detail::toString

#include "open62541_impl.h"

using namespace opcua;

TEST_CASE("TypeWrapper") {
    SUBCASE("Constructor empty") {
        CHECK_NOTHROW(TypeWrapper<UA_Boolean, UA_TYPES_BOOLEAN>());
    }

    SUBCASE("Constructor with native type") {
        UA_Boolean value{true};
        TypeWrapper<UA_Boolean, UA_TYPES_BOOLEAN> wrapper(value);
        CHECK(*wrapper.handle() == true);
    }

    SUBCASE("Constructor with native type (implicit)") {
        TypeWrapper<UA_Int32, UA_TYPES_INT32> wrapper = UA_Int32{123};
        CHECK(*wrapper.handle() == 123);
    }

    SUBCASE("Constructor with wrapper type") {
        TypeWrapper<UA_Double, UA_TYPES_DOUBLE> wrapper(11.11);
        CHECK(*wrapper.handle() == 11.11);
    }

    SUBCASE("Copy constructor") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper(UA_STRING_ALLOC("test"));
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapperConstructor(wrapper);

        CHECK(wrapperConstructor.handle()->data != wrapper.handle()->data);
        CHECK(detail::toString(*wrapperConstructor.handle()) == "test");
    }

    SUBCASE("Copy assignment") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper(UA_STRING_ALLOC("test"));
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapperAssignment = wrapper;

        CHECK(wrapperAssignment.handle()->data != wrapper.handle()->data);
        CHECK(detail::toString(*wrapperAssignment.handle()) == "test");

        // self assignment
        CHECK_NOTHROW(wrapper = wrapper);
    }

    SUBCASE("Copy assignment with native type") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper(UA_STRING_ALLOC("overwrite"));
        UA_String str = UA_STRING_ALLOC("test");
        wrapper = str;
        CHECK(detail::toString(*wrapper.handle()) == "test");
        UA_String_clear(&str);
    }

    SUBCASE("Move constructor") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper(UA_STRING_ALLOC("test"));
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapperConstructor(std::move(wrapper));

        CHECK(wrapper.handle()->data == nullptr);
        CHECK(detail::toString(*wrapperConstructor.handle()) == "test");
    }

    SUBCASE("Move assignment") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper(UA_STRING_ALLOC("test"));
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapperAssignment = std::move(wrapper);

        CHECK(wrapper.handle()->data == nullptr);
        CHECK(detail::toString(*wrapperAssignment.handle()) == "test");

        // self assignment
        CHECK_NOTHROW(wrapper = std::move(wrapper));
    }

    SUBCASE("Move assignment with native type") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper(UA_STRING_ALLOC("overwrite"));
        wrapper = UA_STRING_ALLOC("test");
        CHECK(detail::toString(*wrapper.handle()) == "test");
    }

    SUBCASE("Implicit conversion") {
        TypeWrapper<UA_Float, UA_TYPES_FLOAT> wrapper(11.11f);

        float& value = wrapper;
        CHECK(value == 11.11f);

        const float& cvalue = wrapper;
        CHECK(cvalue == 11.11f);
    }

    SUBCASE("Member access") {
        TypeWrapper<UA_NodeId, UA_TYPES_NODEID> wrapper(UA_NODEID_NUMERIC(1, 1000));
        CHECK(wrapper->namespaceIndex == 1);
        CHECK(wrapper->identifierType == UA_NODEIDTYPE_NUMERIC);
        CHECK(wrapper->identifier.numeric == 1000);
    }

    SUBCASE("Swap") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper1(UA_STRING_ALLOC("test"));
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper2;

        CHECK(wrapper1.handle()->data != nullptr);
        CHECK(wrapper2.handle()->data == nullptr);
        CHECK(detail::toString(*wrapper1.handle()) == "test");

        wrapper1.swap(wrapper2);
        CHECK(wrapper1.handle()->data == nullptr);
        CHECK(wrapper2.handle()->data != nullptr);
        CHECK(detail::toString(*wrapper2.handle()) == "test");
    }

    SUBCASE("Swap with native object") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper;
        UA_String str = UA_STRING_ALLOC("test");

        CHECK(wrapper.handle()->data == nullptr);
        CHECK(str.data != nullptr);

        wrapper.swap(str);
        CHECK(wrapper.handle()->data != nullptr);
        CHECK(str.data == nullptr);

        // UA_String_clear not necessary, because data is now owned by wrapper
    }
}

TEST_CASE("asWrapper / asNative") {
    class Int32Wrapper : public TypeWrapper<int32_t, UA_TYPES_INT32> {
    public:
        using TypeWrapperBase::TypeWrapperBase;

        void increment() {
            auto& ref = *handle();
            ++ref;
        }

        int32_t get() const {
            return *handle();
        }
    };

    SUBCASE("asWrapper") {
        SUBCASE("non-const") {
            int32_t value = 1;
            Int32Wrapper& wrapper = asWrapper<Int32Wrapper>(value);
            wrapper.increment();
            CHECK(value == 2);
            CHECK(wrapper.get() == 2);
        }

        SUBCASE("const") {
            const int32_t value = 1;
            const Int32Wrapper& wrapper = asWrapper<Int32Wrapper>(value);
            CHECK(wrapper.get() == 1);
        }
    }

    SUBCASE("asNative") {
        SUBCASE("non-const") {
            Int32Wrapper wrapper(1);
            int32_t& native = asNative(wrapper);
            native++;
            CHECK(native == 2);
            CHECK(wrapper.get() == 2);
        }

        SUBCASE("non-const") {
            const Int32Wrapper wrapper(1);
            const int32_t& native = asNative(wrapper);
            CHECK(native == 1);
        }
    }
}
