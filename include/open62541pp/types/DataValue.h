#pragma once

#include <cstdint>
#include <optional>

#include "open62541pp/TypeWrapper.h"
#include "open62541pp/open62541.h"
#include "open62541pp/types/DateTime.h"
#include "open62541pp/types/Variant.h"

namespace opcua {

/**
 * UA_DataValue wrapper class.
 * @see https://reference.opcfoundation.org/v104/Core/docs/Part4/7.7
 * @ingroup TypeWrapper
 */
class DataValue : public TypeWrapper<UA_DataValue, UA_TYPES_DATAVALUE> {
public:
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    DataValue(
        const Variant& value,
        std::optional<DateTime> sourceTimestamp,  // NOLINT
        std::optional<DateTime> serverTimestamp,  // NOLINT
        std::optional<uint16_t> sourcePicoseconds,
        std::optional<uint16_t> serverPicoseconds,
        std::optional<UA_StatusCode> statusCode
    );

    /// Get value as pointer (might be `nullptr` if not set).
    Variant* getValuePtr() noexcept;
    /// Get value as pointer (might be `nullptr` if not set).
    const Variant* getValuePtr() const noexcept;

    /// Get value.
    std::optional<Variant> getValue() const;
    /// Get source timestamp for the value.
    std::optional<DateTime> getSourceTimestamp() const;
    /// Get server timestamp for the value.
    std::optional<DateTime> getServerTimestamp() const;
    /// Get picoseconds interval added to the source timestamp.
    std::optional<uint16_t> getSourcePicoseconds() const;
    /// Get picoseconds interval added to the server timestamp.
    std::optional<uint16_t> getServerPicoseconds() const;
    /// Get status code.
    std::optional<UA_StatusCode> getStatusCode() const;

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
    void setStatusCode(UA_StatusCode statusCode);
};

}  // namespace opcua
