#pragma once

#include <memory>
#include <vector>

#include "open62541pp/DataType.h"

// forward declare
struct UA_DataTypeArray;

namespace opcua {

class CustomDataTypes {
public:
    explicit CustomDataTypes(const UA_DataTypeArray** arrayConfig);

    void setCustomDataTypes(std::vector<DataType> dataTypes);

private:
    const UA_DataTypeArray** arrayConfig_;
    std::unique_ptr<UA_DataTypeArray> array_;
    std::vector<DataType> dataTypes_;
};

}  // namespace opcua
