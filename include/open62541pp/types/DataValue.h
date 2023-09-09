#pragma once

#include <cstdint>
#include <optional>
#include <utility>  // forward

#include "open62541pp/TypeWrapper.h"
#include "open62541pp/open62541.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/DateTime.h"
#include "open62541pp/types/Variant.h"

namespace opcua {

/**
 * UA_DataValue wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.11
 * @ingroup TypeWrapper
 */
class DataValue : public TypeWrapper<UA_DataValue, UA_TYPES_DATAVALUE> {
public:
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit constructors

    DataValue(
        Variant value,
        std::optional<DateTime> sourceTimestamp,  // NOLINT
        std::optional<DateTime> serverTimestamp,  // NOLINT
        std::optional<uint16_t> sourcePicoseconds,
        std::optional<uint16_t> serverPicoseconds,
        std::optional<StatusCode> statusCode
    );

    /// Create Variant from scalar value.
    /// @see Variant::fromScalar
    template <typename... Args>
    static DataValue fromScalar(Args&&... args);

    /// Create Variant from array.
    /// @see Variant::fromArray
    template <typename... Args>
    static DataValue fromArray(Args&&... args);

    bool hasValue() const noexcept;
    bool hasSourceTimestamp() const noexcept;
    bool hasServerTimestamp() const noexcept;
    bool hasSourcePicoseconds() const noexcept;
    bool hasServerPicoseconds() const noexcept;
    bool hasStatusCode() const noexcept;

    /// Get value.
    Variant& getValue() noexcept;
    /// Get value.
    const Variant& getValue() const noexcept;
    /// Get source timestamp for the value.
    DateTime getSourceTimestamp() const noexcept;
    /// Get server timestamp for the value.
    DateTime getServerTimestamp() const noexcept;
    /// Get picoseconds interval added to the source timestamp.
    uint16_t getSourcePicoseconds() const noexcept;
    /// Get picoseconds interval added to the server timestamp.
    uint16_t getServerPicoseconds() const noexcept;
    /// Get status code.
    StatusCode getStatusCode() const noexcept;

    /// Set value (copy).
    void setValue(const Variant& value);
    /// Set value (move).
    void setValue(Variant&& value);
    /// Set source timestamp for the value.
    void setSourceTimestamp(DateTime sourceTimestamp);
    /// Set server timestamp for the value.
    void setServerTimestamp(DateTime serverTimestamp);
    /// Set picoseconds interval added to the source timestamp.
    void setSourcePicoseconds(uint16_t sourcePicoseconds);
    /// Set picoseconds interval added to the server timestamp.
    void setServerPicoseconds(uint16_t serverPicoseconds);
    /// Set status code.
    void setStatusCode(StatusCode statusCode);
};

/* --------------------------------------- Implementation --------------------------------------- */

template <typename... Args>
DataValue DataValue::fromScalar(Args&&... args) {
    DataValue dv{};
    asWrapper<Variant>(dv->value) = Variant::fromScalar(std::forward<Args>(args)...);
    dv->hasValue = true;
    return dv;
}

template <typename... Args>
DataValue DataValue::fromArray(Args&&... args) {
    DataValue dv{};
    asWrapper<Variant>(dv->value) = Variant::fromArray(std::forward<Args>(args)...);
    dv->hasValue = true;
    return dv;
}

}  // namespace opcua
