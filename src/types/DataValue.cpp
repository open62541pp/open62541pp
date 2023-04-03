#include "open62541pp/types/DataValue.h"

#include "../open62541_impl.h"

namespace opcua {

DataValue::DataValue(
    const std::optional<Variant>& value,
    const std::optional<DateTime>& sourceTimestamp,
    const std::optional<DateTime>& serverTimestamp,
    std::optional<uint16_t> sourcePicoseconds,
    std::optional<uint16_t> serverPicoseconds,
    std::optional<UA_StatusCode> statusCode
)
    : DataValue(UA_DataValue{
          {},
          sourceTimestamp.value_or(DateTime{}),
          serverTimestamp.value_or(DateTime{}),
          sourcePicoseconds.value_or(uint16_t{}),
          serverPicoseconds.value_or(uint16_t{}),
          statusCode.value_or(UA_StatusCode{}),
          value.has_value(),
          sourceTimestamp.has_value(),
          serverTimestamp.has_value(),
          sourcePicoseconds.has_value(),
          serverPicoseconds.has_value(),
          statusCode.has_value(),
      }) {
    if (value.has_value()) {
        UA_Variant_copy(value.value().handle(), &handle()->value);
    }
}

std::optional<Variant> DataValue::getValue() const {
    if (handle()->hasValue) {
        return Variant(handle()->value);
    }
    return {};
}

std::optional<DateTime> DataValue::getSourceTimestamp() const {
    if (handle()->hasSourceTimestamp) {
        return DateTime(handle()->sourceTimestamp);
    }
    return {};
}

std::optional<DateTime> DataValue::getServerTimestamp() const {
    if (handle()->hasServerTimestamp) {
        return DateTime(handle()->serverTimestamp);
    }
    return {};
}

std::optional<uint16_t> DataValue::getSourcePicoseconds() const {
    if (handle()->hasSourcePicoseconds) {
        return handle()->sourcePicoseconds;
    }
    return {};
}

std::optional<uint16_t> DataValue::getServerPicoseconds() const {
    if (handle()->hasSourcePicoseconds) {
        return handle()->serverPicoseconds;
    }
    return {};
}

std::optional<uint32_t> DataValue::getStatusCode() const {
    if (handle()->hasStatus) {
        return handle()->status;
    }
    return {};
}

}  // namespace opcua
