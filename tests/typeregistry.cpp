#include <catch2/catch_test_macros.hpp>

#include "open62541pp/typeregistry.hpp"
#include "open62541pp/ua/typeregistry.hpp"
#include "open62541pp/wrapper.hpp"

using namespace opcua;

namespace {
struct Custom {
    UA_String string;
};
}  // namespace

namespace opcua {
template <>
struct TypeRegistry<Custom> {
    static const auto& getDataType() noexcept {
        return UA_TYPES[UA_TYPES_STRING];
    }
};
}  // namespace opcua

TEST_CASE("TypeRegistry") {
    SECTION("Builtin") {
        CHECK(IsRegistered<float>::value);
        CHECK_FALSE(IsRegistered<const volatile float>::value);
        CHECK_FALSE(IsRegistered<float*>::value);
        CHECK_FALSE(IsRegistered<float&>::value);

        CHECK(&TypeRegistry<float>::getDataType() == &UA_TYPES[UA_TYPES_FLOAT]);
        CHECK(&getDataType<float>() == &UA_TYPES[UA_TYPES_FLOAT]);
        CHECK(&getDataType<const volatile float>() == &UA_TYPES[UA_TYPES_FLOAT]);
    }

    SECTION("Generated") {
        CHECK(&TypeRegistry<UA_AddNodesItem>::getDataType() == &UA_TYPES[UA_TYPES_ADDNODESITEM]);
        CHECK(&getDataType<UA_AddNodesItem>() == &UA_TYPES[UA_TYPES_ADDNODESITEM]);
    }

    SECTION("Custom") {
        CHECK(&TypeRegistry<Custom>::getDataType() == &UA_TYPES[UA_TYPES_STRING]);
        CHECK(&getDataType<Custom>() == &UA_TYPES[UA_TYPES_STRING]);
    }

    SECTION("Wrapper") {
        class CustomWrapper : public Wrapper<Custom> {};

        CHECK(&TypeRegistry<CustomWrapper>::getDataType() == &UA_TYPES[UA_TYPES_STRING]);
        CHECK(&getDataType<CustomWrapper>() == &UA_TYPES[UA_TYPES_STRING]);
    }
}
