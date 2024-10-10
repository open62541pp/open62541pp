#include <doctest/doctest.h>

#include "open62541pp/datatype.hpp"
#include "open62541pp/detail/open62541/common.h"

#include "datatypearray.hpp"

using namespace opcua;

TEST_CASE("DataTypeArray") {
    DataTypeArray array({
        DataType{UA_TYPES[UA_TYPES_INT32]},
        DataType{UA_TYPES[UA_TYPES_FLOAT]},
        DataType{UA_TYPES[UA_TYPES_STRING]},
    });

    const UA_DataTypeArray* native = array.handle();
    CHECK(native != nullptr);
    CHECK(native->next == nullptr);
    CHECK(native->typesSize == 3);
    CHECK(native->types[0] == UA_TYPES[UA_TYPES_INT32]);
    CHECK(native->types[1] == UA_TYPES[UA_TYPES_FLOAT]);
    CHECK(native->types[2] == UA_TYPES[UA_TYPES_STRING]);
}
