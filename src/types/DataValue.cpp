#include "open62541pp/types/DataValue.h"

#include "../open62541_impl.h"

namespace opcua {

static UA_DataValue fromOptionals(
    const std::optional<DateTime>& sourceTimestamp,
    const std::optional<DateTime>& serverTimestamp,
    const std::optional<uint16_t>& sourcePicoseconds,
    const std::optional<uint16_t>& serverPicoseconds,
    const std::optional<StatusCode>& statusCode
) {
    return {
        {},
        sourceTimestamp.value_or(DateTime{}),
        serverTimestamp.value_or(DateTime{}),
        sourcePicoseconds.value_or(uint16_t{}),
        serverPicoseconds.value_or(uint16_t{}),
        statusCode.value_or(StatusCode{}),
        false,
        sourceTimestamp.has_value(),
        serverTimestamp.has_value(),
        sourcePicoseconds.has_value(),
        serverPicoseconds.has_value(),
        statusCode.has_value(),
    };
}

DataValue::DataValue(
    Variant value,
    std::optional<DateTime> sourceTimestamp,  // NOLINT
    std::optional<DateTime> serverTimestamp,  // NOLINT
    std::optional<uint16_t> sourcePicoseconds,
    std::optional<uint16_t> serverPicoseconds,
    std::optional<StatusCode> statusCode
)
    : DataValue(fromOptionals(
          sourceTimestamp, serverTimestamp, sourcePicoseconds, serverPicoseconds, statusCode
      )) {
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

Variant& DataValue::getValue() noexcept {
    return asWrapper<Variant>(handle()->value);
}

const Variant& DataValue::getValue() const noexcept {
    return asWrapper<Variant>(handle()->value);
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
    UA_Variant_copy(value.handle(), &handle()->value);
    handle()->hasValue = true;
}

void DataValue::setValue(Variant&& value) {
    value.swap(handle()->value);
    handle()->hasValue = true;
}

void DataValue::setSourceTimestamp(DateTime sourceTimestamp) {
    handle()->sourceTimestamp = *sourceTimestamp.handle();
    handle()->hasSourceTimestamp = true;
}

void DataValue::setServerTimestamp(DateTime serverTimestamp) {
    handle()->serverTimestamp = *serverTimestamp.handle();
    handle()->hasServerTimestamp = true;
}

void DataValue::setSourcePicoseconds(uint16_t sourcePicoseconds) {
    handle()->sourcePicoseconds = sourcePicoseconds;
    handle()->hasSourcePicoseconds = true;
}

void DataValue::setServerPicoseconds(uint16_t serverPicoseconds) {
    handle()->serverPicoseconds = serverPicoseconds;
    handle()->hasServerPicoseconds = true;
}

void DataValue::setStatusCode(StatusCode statusCode) {
    handle()->status = statusCode;
    handle()->hasStatus = true;
}

}  // namespace opcua
