#include "open62541pp/DataValue.h"

namespace opcua {

void DataValue::setVariant(const Variant& variant) {
    this->handle()->value = *variant.handle();
    this->handle()->hasValue = true;
}

Variant DataValue::getVariant() const {
    return Variant(this->handle()->value);
}

void DataValue::setSourceTimeStamp(const DateTime& ts) {
    this->handle()->sourceTimestamp = *ts.handle();
    this->handle()->hasSourceTimestamp = true;
}

DateTime DataValue::getSourceTimeStamp() const {
    if (this->handle()->hasSourceTimestamp) {
        return DateTime(this->handle()->sourceTimestamp);
    }
    return DateTime();
}

void DataValue::setStatusCode(const UA_StatusCode code) {
    this->handle()->status = code;
}

UA_StatusCode DataValue::getStatusCode() const {
    return this->handle()->status;
}

}  // namespace opcua
