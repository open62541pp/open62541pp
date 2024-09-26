#include <doctest/doctest.h>

#include "open62541pp/datatype.hpp"
#include "open62541pp/detail/open62541/common.h"

#include "customdatatypes.hpp"

using namespace opcua;

TEST_CASE("CustomDataTypes") {
    const UA_DataTypeArray* dataTypeArray{};
    CHECK(dataTypeArray == nullptr);

    CustomDataTypes customDataTypes(dataTypeArray);
    customDataTypes.assign({
        DataType{UA_TYPES[UA_TYPES_INT32]},
        DataType{UA_TYPES[UA_TYPES_FLOAT]},
        DataType{UA_TYPES[UA_TYPES_STRING]},
    });

    CHECK(dataTypeArray != nullptr);
    CHECK(dataTypeArray->next == nullptr);
    CHECK(dataTypeArray->typesSize == 3);
    CHECK(dataTypeArray->types[0] == UA_TYPES[UA_TYPES_INT32]);
    CHECK(dataTypeArray->types[1] == UA_TYPES[UA_TYPES_FLOAT]);
    CHECK(dataTypeArray->types[2] == UA_TYPES[UA_TYPES_STRING]);
}
