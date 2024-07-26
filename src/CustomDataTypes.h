#pragma once

#include <memory>
#include <utility>  // move
#include <vector>

#include "open62541pp/DataType.h"
#include "open62541pp/Wrapper.h"  // asNative
#include "open62541pp/detail/open62541/common.h"

namespace opcua {

class CustomDataTypes {
public:
    explicit CustomDataTypes(const UA_DataTypeArray*& array)
        : wrapped_(array) {}

    void assign(std::vector<DataType> dataTypes) {
        dataTypes_ = std::move(dataTypes);
        // NOLINTNEXTLINE
        array_ = std::unique_ptr<UA_DataTypeArray>(new UA_DataTypeArray{
            nullptr,  // next
            dataTypes_.size(),
            asNative(dataTypes_.data()),
        });
        wrapped_ = array_.get();
    }

private:
    const UA_DataTypeArray*& wrapped_;
    std::vector<DataType> dataTypes_;
    std::unique_ptr<UA_DataTypeArray> array_;
};

}  // namespace opcua
