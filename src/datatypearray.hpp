#pragma once

#include "open62541pp/datatype.hpp"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/span.hpp"
#include "open62541pp/wrapper.hpp"  // asNative

namespace opcua {

class DataTypeArray : public Wrapper<UA_DataTypeArray> {
public:
    using Wrapper::Wrapper;

    explicit DataTypeArray(Span<const DataType> types, const UA_DataTypeArray* next = nullptr)
        : Wrapper(UA_DataTypeArray{
              next,
              types.size(),
              asNative(types.data()),
#if UAPP_OPEN62541_VER_GE(1, 4)
              false,  // cleanup
#endif
          }) {
    }
};

}  // namespace opcua
