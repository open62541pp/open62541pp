#include <catch2/catch_test_macros.hpp>

#include "open62541pp/typeregistry.hpp"
#include "open62541pp/typewrapper.hpp"
#include "open62541pp/ua/typeregistry.hpp"

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
        CHECK(detail::IsRegistered<float>::value);
        CHECK_FALSE(detail::IsRegistered<const volatile float>::value);
        CHECK_FALSE(detail::IsRegistered<float*>::value);
        CHECK_FALSE(detail::IsRegistered<float&>::value);

        CHECK(&TypeRegistry<float>::getDataType() == &UA_TYPES[UA_TYPES_FLOAT]);
        CHECK(&getDataType<float>() == &UA_TYPES[UA_TYPES_FLOAT]);
        CHECK(&getDataType<const volatile float>() == &UA_TYPES[UA_TYPES_FLOAT]);
    }

    SECTION("Generated") {
        CHECK(&TypeRegistry<UA_AddNodesItem>::getDataType() == &UA_TYPES[UA_TYPES_ADDNODESITEM]);
        CHECK(&getDataType<UA_AddNodesItem>() == &UA_TYPES[UA_TYPES_ADDNODESITEM]);
    }

    SECTION("TypeWrapper") {
        class Wrapper : public TypeWrapper<float, UA_TYPES_FLOAT> {};

        CHECK(&TypeRegistry<Wrapper>::getDataType() == &UA_TYPES[UA_TYPES_FLOAT]);
        CHECK(&getDataType<Wrapper>() == &UA_TYPES[UA_TYPES_FLOAT]);
    }

    SECTION("Custom") {
        CHECK(&TypeRegistry<Custom>::getDataType() == &UA_TYPES[UA_TYPES_STRING]);
        CHECK(&getDataType<Custom>() == &UA_TYPES[UA_TYPES_STRING]);
    }
}
