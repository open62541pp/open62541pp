#include <doctest/doctest.h>

#include "open62541pp/TypeRegistry.h"
#include "open62541pp/TypeWrapper.h"

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
    SUBCASE("Builtin") {
        CHECK(detail::isRegisteredType<float>);
        CHECK_FALSE(detail::isRegisteredType<const volatile float>);
        CHECK_FALSE(detail::isRegisteredType<float*>);
        CHECK_FALSE(detail::isRegisteredType<float&>);

        CHECK(&TypeRegistry<float>::getDataType() == &UA_TYPES[UA_TYPES_FLOAT]);
        CHECK(&detail::getDataType<float>() == &UA_TYPES[UA_TYPES_FLOAT]);
        CHECK(&detail::getDataType<const volatile float>() == &UA_TYPES[UA_TYPES_FLOAT]);
    }

    SUBCASE("Generated") {
        CHECK(&TypeRegistry<UA_AddNodesItem>::getDataType() == &UA_TYPES[UA_TYPES_ADDNODESITEM]);
        CHECK(&detail::getDataType<UA_AddNodesItem>() == &UA_TYPES[UA_TYPES_ADDNODESITEM]);
    }

    SUBCASE("TypeWrapper") {
        class Wrapper : public TypeWrapper<float, UA_TYPES_FLOAT> {};

        CHECK(&TypeRegistry<Wrapper>::getDataType() == &UA_TYPES[UA_TYPES_FLOAT]);
        CHECK(&detail::getDataType<Wrapper>() == &UA_TYPES[UA_TYPES_FLOAT]);
    }

    SUBCASE("Custom") {
        CHECK(&TypeRegistry<Custom>::getDataType() == &UA_TYPES[UA_TYPES_STRING]);
        CHECK(&detail::getDataType<Custom>() == &UA_TYPES[UA_TYPES_STRING]);
    }
}
