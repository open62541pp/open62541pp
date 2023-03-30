#pragma once

#include "open62541pp/TypeWrapper.h"
#include "open62541pp/Variant.h"

namespace opcua {

class DataValue : public TypeWrapper<UA_DataValue, UA_TYPES_DATAVALUE> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    void setVariant(const Variant& variant);
    Variant getVariant() const;

    void setSourceTimeStamp(const DateTime& ts);
    DateTime getSourceTimeStamp() const;

    void setStatusCode(const UA_StatusCode);
    UA_StatusCode getStatusCode() const;
};

}  // namespace opcua
