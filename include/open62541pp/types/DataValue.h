#pragma once

#include <cstdint>
#include <optional>

#include "open62541pp/Common.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/open62541.h"
#include "open62541pp/types/Variant.h"

namespace opcua {

/**
 * UA_DataValue wrapper class.
 * @ingroup TypeWrapper
 * @todo Prevent unnecessary copies, maybe just use a struct with wrapped types?
 */
class DataValue : public TypeWrapper<UA_DataValue, UA_TYPES_DATAVALUE> {
public:
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    DataValue(
        const std::optional<Variant>& value,
        const std::optional<DateTime>& sourceTimestamp,
        const std::optional<DateTime>& serverTimestamp,
        std::optional<uint16_t> sourcePicoseconds,
        std::optional<uint16_t> serverPicoseconds,
        std::optional<UA_StatusCode> statusCode
    );

    std::optional<Variant> getValue() const;
    std::optional<DateTime> getSourceTimestamp() const;
    std::optional<DateTime> getServerTimestamp() const;
    std::optional<uint16_t> getSourcePicoseconds() const;
    std::optional<uint16_t> getServerPicoseconds() const;
    std::optional<UA_StatusCode> getStatusCode() const;
};

}  // namespace opcua
