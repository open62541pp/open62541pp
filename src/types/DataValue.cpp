#include "open62541pp/types/DataValue.h"

namespace opcua {

DataValue::DataValue(
    Variant value,
    std::optional<DateTime> sourceTimestamp,  // NOLINT
    std::optional<DateTime> serverTimestamp,  // NOLINT
    std::optional<uint16_t> sourcePicoseconds,
    std::optional<uint16_t> serverPicoseconds,
    std::optional<StatusCode> statusCode
) noexcept
    : DataValue(UA_DataValue{
          UA_Variant{},
          sourceTimestamp.value_or(UA_DateTime{}),
          serverTimestamp.value_or(UA_DateTime{}),
          sourcePicoseconds.value_or(uint16_t{}),
          serverPicoseconds.value_or(uint16_t{}),
          statusCode.value_or(UA_StatusCode{}),
          false,
          sourceTimestamp.has_value(),
          serverTimestamp.has_value(),
          sourcePicoseconds.has_value(),
          serverPicoseconds.has_value(),
          statusCode.has_value(),
      }) {
    setValue(std::move(value));
}

bool DataValue::hasValue() const noexcept {
    return handle()->hasValue;
}

bool DataValue::hasSourceTimestamp() const noexcept {
    return handle()->hasSourceTimestamp;
}

bool DataValue::hasServerTimestamp() const noexcept {
    return handle()->hasServerTimestamp;
}

bool DataValue::hasSourcePicoseconds() const noexcept {
    return handle()->hasSourcePicoseconds;
}

bool DataValue::hasServerPicoseconds() const noexcept {
    return handle()->hasServerPicoseconds;
}

bool DataValue::hasStatusCode() const noexcept {
    return handle()->hasStatus;
}

DateTime DataValue::getSourceTimestamp() const noexcept {
    return DateTime(handle()->sourceTimestamp);  // NOLINT
}

DateTime DataValue::getServerTimestamp() const noexcept {
    return DateTime(handle()->serverTimestamp);  // NOLINT
}

uint16_t DataValue::getSourcePicoseconds() const noexcept {
    return handle()->sourcePicoseconds;
}

uint16_t DataValue::getServerPicoseconds() const noexcept {
    return handle()->serverPicoseconds;
}

StatusCode DataValue::getStatusCode() const noexcept {
    return handle()->status;
}

void DataValue::setValue(const Variant& value) {
    asWrapper<Variant>(handle()->value) = value;
    handle()->hasValue = true;
}

void DataValue::setValue(Variant&& value) noexcept {
    asWrapper<Variant>(handle()->value) = std::move(value);
    handle()->hasValue = true;
}

void DataValue::setSourceTimestamp(DateTime sourceTimestamp) noexcept {
    handle()->sourceTimestamp = *sourceTimestamp.handle();
    handle()->hasSourceTimestamp = true;
}

void DataValue::setServerTimestamp(DateTime serverTimestamp) noexcept {
    handle()->serverTimestamp = *serverTimestamp.handle();
    handle()->hasServerTimestamp = true;
}

void DataValue::setSourcePicoseconds(uint16_t sourcePicoseconds) noexcept {
    handle()->sourcePicoseconds = sourcePicoseconds;
    handle()->hasSourcePicoseconds = true;
}

void DataValue::setServerPicoseconds(uint16_t serverPicoseconds) noexcept {
    handle()->serverPicoseconds = serverPicoseconds;
    handle()->hasServerPicoseconds = true;
}

void DataValue::setStatusCode(StatusCode statusCode) noexcept {
    handle()->status = statusCode;
    handle()->hasStatus = true;
}

}  // namespace opcua
