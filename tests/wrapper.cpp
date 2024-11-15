#include <utility>  // move

#include <doctest/doctest.h>

#include "open62541pp/detail/string_utils.hpp"  // detail::toString
#include "open62541pp/typewrapper.hpp"

using namespace opcua;

TEST_CASE("Wrapper") {
    SUBCASE("Default constructor") {
        Wrapper<int> wrapper;
        CHECK(*wrapper.handle() == 0);
    }

    SUBCASE("Copy constructor with native type") {
        int native = 11;
        Wrapper<int> wrapper(native);
        CHECK(*wrapper.handle() == 11);
    }

    SUBCASE("Move constructor with native type") {
        Wrapper<int> wrapper(11);
        CHECK(*wrapper.handle() == 11);
    }

    SUBCASE("Copy assignment with native type") {
        Wrapper<int> wrapper;
        int native = 11;
        wrapper = native;
        CHECK(*wrapper.handle() == 11);
    }

    SUBCASE("Move assignment with native type") {
        Wrapper<int> wrapper;
        wrapper = 11;
        CHECK(*wrapper.handle() == 11);
    }

    SUBCASE("Implicit conversion to native type") {
        Wrapper<int> wrapper(11);
        {
            int& native = wrapper;
            CHECK(native == 11);
        }
        {
            const int& native = wrapper;
            CHECK(native == 11);
        }
    }

    SUBCASE("Swap with wrapper object") {
        Wrapper<int> wrapper1(1);
        Wrapper<int> wrapper2(2);
        wrapper1.swap(wrapper2);
        CHECK(*wrapper1.handle() == 2);
        CHECK(*wrapper2.handle() == 1);
    }

    SUBCASE("Swap with native object") {
        Wrapper<int> wrapper(1);
        int native = 2;
        wrapper.swap(native);
        CHECK(*wrapper.handle() == 2);
        CHECK(native == 1);
    }

    SUBCASE("Member access") {
        struct S {
            int value;
        };

        Wrapper<S> wrapper(S{11});
        CHECK(wrapper->value == 11);
    }
}

TEST_CASE("asWrapper / asNative") {
    class Int32Wrapper : public Wrapper<int32_t> {
    public:
        using Wrapper<int32_t>::Wrapper;

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

TEST_CASE("TypeWrapper") {
    SUBCASE("Copy constructor") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper(UA_STRING_ALLOC("test"));
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapperConstructor(wrapper);

        CHECK(wrapperConstructor.handle()->data != wrapper.handle()->data);
        CHECK(detail::toString(*wrapperConstructor.handle()) == "test");
    }

    SUBCASE("Copy constructor with native type") {
        double value = 11.11;
        TypeWrapper<double, UA_TYPES_DOUBLE> wrapper(value);
        CHECK(*wrapper.handle() == 11.11);
    }

    SUBCASE("Move Constructor with native type") {
        TypeWrapper<double, UA_TYPES_DOUBLE> wrapper(11.11);
        CHECK(*wrapper.handle() == 11.11);
    }

    SUBCASE("Move constructor") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper(UA_STRING_ALLOC("test"));
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapperConstructor(std::move(wrapper));

        CHECK(wrapper.handle()->data == nullptr);
        CHECK(detail::toString(*wrapperConstructor.handle()) == "test");
    }

    SUBCASE("Copy assignment") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper(UA_STRING_ALLOC("test"));
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapperAssignment = wrapper;

        CHECK(wrapperAssignment.handle()->data != wrapper.handle()->data);
        CHECK(detail::toString(*wrapperAssignment.handle()) == "test");
    }

    SUBCASE("Copy assignment with native type") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper(UA_STRING_ALLOC("overwrite"));
        UA_String str = UA_STRING_ALLOC("test");
        wrapper = str;
        CHECK(detail::toString(*wrapper.handle()) == "test");
        UA_clear(&str, &UA_TYPES[UA_TYPES_STRING]);
    }

    SUBCASE("Move assignment") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper(UA_STRING_ALLOC("test"));
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapperAssignment = std::move(wrapper);

        CHECK(wrapper.handle()->data == nullptr);
        CHECK(detail::toString(*wrapperAssignment.handle()) == "test");
    }

    SUBCASE("Move assignment with native type") {
        TypeWrapper<UA_String, UA_TYPES_STRING> wrapper(UA_STRING_ALLOC("overwrite"));
        wrapper = UA_STRING_ALLOC("test");
        CHECK(detail::toString(*wrapper.handle()) == "test");
    }
}
