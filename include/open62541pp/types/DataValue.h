#pragma once

#include <cstdint>
#include <optional>
#include <utility>  // forward

#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/DateTime.h"
#include "open62541pp/types/Variant.h"

namespace opcua {

/**
 * UA_DataValue wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.11
 * @ingroup Wrapper
 */
class DataValue : public TypeWrapper<UA_DataValue, UA_TYPES_DATAVALUE> {
public:
    using TypeWrapperBase::TypeWrapperBase;  // inherit constructors

    explicit DataValue(Variant value) noexcept {
        setValue(std::move(value));
    }

    DataValue(
        Variant value,
        std::optional<DateTime> sourceTimestamp,  // NOLINT
        std::optional<DateTime> serverTimestamp,  // NOLINT
        std::optional<uint16_t> sourcePicoseconds,
        std::optional<uint16_t> serverPicoseconds,
        std::optional<StatusCode> status
    ) noexcept
        : DataValue(UA_DataValue{
              UA_Variant{},
              sourceTimestamp.value_or(UA_DateTime{}),
              serverTimestamp.value_or(UA_DateTime{}),
              sourcePicoseconds.value_or(uint16_t{}),
              serverPicoseconds.value_or(uint16_t{}),
              status.value_or(UA_StatusCode{}),
              false,
              sourceTimestamp.has_value(),
              serverTimestamp.has_value(),
              sourcePicoseconds.has_value(),
              serverPicoseconds.has_value(),
              status.has_value(),
          }) {
        setValue(std::move(value));
    }

    /// Create DataValue from scalar value.
    /// @see Variant::fromScalar
    template <VariantPolicy Policy = VariantPolicy::Copy, typename... Args>
    [[nodiscard]] static DataValue fromScalar(Args&&... args) {
        return DataValue(Variant::fromScalar<Policy>(std::forward<Args>(args)...));
    }

    /// Create DataValue from array.
    /// @see Variant::fromArray
    template <VariantPolicy Policy = VariantPolicy::Copy, typename... Args>
    [[nodiscard]] static DataValue fromArray(Args&&... args) {
        return DataValue(Variant::fromArray<Policy>(std::forward<Args>(args)...));
    }

    bool hasValue() const noexcept {
        return handle()->hasValue;
    }

    bool hasSourceTimestamp() const noexcept {
        return handle()->hasSourceTimestamp;
    }

    bool hasServerTimestamp() const noexcept {
        return handle()->hasServerTimestamp;
    }

    bool hasSourcePicoseconds() const noexcept {
        return handle()->hasSourcePicoseconds;
    }

    bool hasServerPicoseconds() const noexcept {
        return handle()->hasServerPicoseconds;
    }

    bool hasStatus() const noexcept {
        return handle()->hasStatus;
    }

    [[deprecated("Use hasStatus() instead")]]
    bool hasStatusCode() const noexcept {
        return hasStatus();
    }

    /// Get value.
    Variant& getValue() & noexcept {
        return asWrapper<Variant>(handle()->value);
    }

    /// Get value.
    const Variant& getValue() const& noexcept {
        return asWrapper<Variant>(handle()->value);
    }

    /// Get value (rvalue).
    Variant&& getValue() && noexcept {
        return std::move(asWrapper<Variant>(handle()->value));
    }

    /// Get value (rvalue).
    const Variant&& getValue() const&& noexcept {
        return std::move(asWrapper<Variant>(handle()->value));  // NOLINT
    }

    /// Get source timestamp for the value.
    DateTime getSourceTimestamp() const noexcept {
        return DateTime(handle()->sourceTimestamp);  // NOLINT
    }

    /// Get server timestamp for the value.
    DateTime getServerTimestamp() const noexcept {
        return DateTime(handle()->serverTimestamp);  // NOLINT
    }

    /// Get picoseconds interval added to the source timestamp.
    uint16_t getSourcePicoseconds() const noexcept {
        return handle()->sourcePicoseconds;
    }

    /// Get picoseconds interval added to the server timestamp.
    uint16_t getServerPicoseconds() const noexcept {
        return handle()->serverPicoseconds;
    }

    /// Get status.
    StatusCode getStatus() const noexcept {
        return handle()->status;
    }

    [[deprecated("Use getStatus() instead")]]
    StatusCode getStatusCode() const noexcept {
        return getStatus();
    }

    /// Set value (copy).
    void setValue(const Variant& value) {
        asWrapper<Variant>(handle()->value) = value;
        handle()->hasValue = true;
    }

    /// Set value (move).
    void setValue(Variant&& value) noexcept {
        asWrapper<Variant>(handle()->value) = std::move(value);
        handle()->hasValue = true;
    }

    /// Set source timestamp for the value.
    void setSourceTimestamp(DateTime sourceTimestamp) noexcept {  // NOLINT
        handle()->sourceTimestamp = sourceTimestamp.get();
        handle()->hasSourceTimestamp = true;
    }

    /// Set server timestamp for the value.
    void setServerTimestamp(DateTime serverTimestamp) noexcept {  // NOLINT
        handle()->serverTimestamp = serverTimestamp.get();
        handle()->hasServerTimestamp = true;
    }

    /// Set picoseconds interval added to the source timestamp.
    void setSourcePicoseconds(uint16_t sourcePicoseconds) noexcept {
        handle()->sourcePicoseconds = sourcePicoseconds;
        handle()->hasSourcePicoseconds = true;
    }

    /// Set picoseconds interval added to the server timestamp.
    void setServerPicoseconds(uint16_t serverPicoseconds) noexcept {
        handle()->serverPicoseconds = serverPicoseconds;
        handle()->hasServerPicoseconds = true;
    }

    /// Set status.
    void setStatus(StatusCode status) noexcept {
        handle()->status = status;
        handle()->hasStatus = true;
    }

    [[deprecated("Use setStatus(StatusCode) instead.")]]
    void setStatusCode(StatusCode statusCode) noexcept {
        setStatus(statusCode);
    }
};

}  // namespace opcua
