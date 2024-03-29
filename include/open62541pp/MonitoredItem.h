#pragma once

#include <cstdint>
#include <type_traits>

#include "open62541pp/Common.h"  // AttributeId
#include "open62541pp/Config.h"
#include "open62541pp/services/MonitoredItem.h"
#include "open62541pp/types/NodeId.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS

namespace opcua {

// forward declarations
class Server;

using MonitoringParametersEx = services::MonitoringParametersEx;
/// @deprecated Use alias MonitoringParametersEx instead
using MonitoringParameters
    [[deprecated("Use alias MonitoringParametersEx instead")]] = MonitoringParametersEx;

/**
 * High-level monitored item class.
 *
 * @tparam Connection Server or Client
 * @note Not all methods are available and implemented for servers.
 *
 * Use the free functions in the opcua::services namespace for more advanced usage:
 * - @ref MonitoredItem
 */
template <typename Connection>
class MonitoredItem {
public:
    /// Wrap an existing monitored item.
    /// The `subscriptionId` is ignored and set to `0U` for servers.
    MonitoredItem(
        Connection& connection, uint32_t subscriptionId, uint32_t monitoredItemId
    ) noexcept
        : connection_(connection),
          subscriptionId_(std::is_same_v<Connection, Server> ? 0U : subscriptionId),
          monitoredItemId_(monitoredItemId) {}

    /// Get the server/client instance.
    Connection& connection() noexcept {
        return connection_;
    }

    /// Get the server/client instance.
    const Connection& connection() const noexcept {
        return connection_;
    }

    /// @deprecated Use connection() instead
    [[deprecated("Use connection() instead")]]
    Connection& getConnection() noexcept {
        return connection_;
    }

    /// @deprecated Use connection() instead
    [[deprecated("Use connection() instead")]]
    const Connection& getConnection() const noexcept {
        return connection_;
    }

    /// Get the server-assigned identifier of the underlying subscription.
    uint32_t subscriptionId() const noexcept {
        return subscriptionId_;
    }

    /// @deprecated Use subscriptionId() instead
    [[deprecated("Use subscriptionId() instead")]]
    uint32_t getSubscriptionId() const noexcept {
        return subscriptionId_;
    }

    /// Get the server-assigned identifier of this monitored item.
    uint32_t monitoredItemId() const noexcept {
        return monitoredItemId_;
    }

    /// @deprecated Use monitoredItemId() instead
    [[deprecated("Use monitoredItemId() instead")]]
    uint32_t getMonitoredItemId() const noexcept {
        return monitoredItemId_;
    }

    /// Get the monitored NodeId.
    const NodeId& getNodeId() const;

    /// Get the monitored AttributeId.
    AttributeId getAttributeId() const;

    /// Modify this monitored item.
    /// @note Not implemented for Server.
    /// @see services::modifyMonitoredItem
    void setMonitoringParameters(MonitoringParametersEx& parameters) {
        services::modifyMonitoredItem(connection_, subscriptionId_, monitoredItemId_, parameters);
    }

    /// Set the monitoring mode of this monitored item.
    /// @note Not implemented for Server.
    /// @see services::setMonitoringMode
    void setMonitoringMode(MonitoringMode monitoringMode) {
        services::setMonitoringMode(connection_, subscriptionId_, monitoredItemId_, monitoringMode);
    }

    /// Delete this monitored item.
    /// @see services::deleteMonitoredItem
    void deleteMonitoredItem() {
        services::deleteMonitoredItem(connection_, subscriptionId_, monitoredItemId_);
    }

private:
    Connection& connection_;
    uint32_t subscriptionId_{0U};
    uint32_t monitoredItemId_{0U};
};

/* ---------------------------------------------------------------------------------------------- */

template <typename T>
inline bool operator==(const MonitoredItem<T>& lhs, const MonitoredItem<T>& rhs) noexcept {
    return (lhs.connection() == rhs.connection()) &&
           (lhs.subscriptionId() == rhs.subscriptionId()) &&
           (lhs.monitoredItemId() == rhs.monitoredItemId());
}

template <typename T>
inline bool operator!=(const MonitoredItem<T>& lhs, const MonitoredItem<T>& rhs) noexcept {
    return !(lhs == rhs);
}

}  // namespace opcua

#endif
