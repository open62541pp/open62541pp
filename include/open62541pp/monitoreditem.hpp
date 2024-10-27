#pragma once

#include <cstdint>
#include <type_traits>

#include "open62541pp/common.hpp"  // AttributeId
#include "open62541pp/config.hpp"
#include "open62541pp/services/monitoreditem.hpp"
#include "open62541pp/types.hpp"

#ifdef UA_ENABLE_SUBSCRIPTIONS

namespace opcua {

class Server;

using MonitoringParametersEx = services::MonitoringParametersEx;

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
        : connection_(&connection),
          subscriptionId_(std::is_same_v<Connection, Server> ? 0U : subscriptionId),
          monitoredItemId_(monitoredItemId) {}

    /// Get the server/client instance.
    Connection& connection() noexcept {
        return *connection_;
    }

    /// Get the server/client instance.
    const Connection& connection() const noexcept {
        return *connection_;
    }

    /// Get the server-assigned identifier of the underlying subscription.
    uint32_t subscriptionId() const noexcept {
        return subscriptionId_;
    }

    /// Get the server-assigned identifier of this monitored item.
    uint32_t monitoredItemId() const noexcept {
        return monitoredItemId_;
    }

    /// Get the monitored NodeId.
    const NodeId& getNodeId();

    /// Get the monitored AttributeId.
    AttributeId getAttributeId();

    /// Modify this monitored item.
    /// @note Not implemented for Server.
    /// @see services::modifyMonitoredItem
    void setMonitoringParameters(const MonitoringParametersEx& parameters) {
        services::modifyMonitoredItem(connection(), subscriptionId(), monitoredItemId(), parameters)
            .getStatusCode()
            .throwIfBad();
    }

    /// Set the monitoring mode of this monitored item.
    /// @note Not implemented for Server.
    /// @see services::setMonitoringMode
    void setMonitoringMode(MonitoringMode monitoringMode) {
        services::setMonitoringMode(
            connection(), subscriptionId(), monitoredItemId(), monitoringMode
        )
            .throwIfBad();
    }

    /// Delete this monitored item.
    /// @see services::deleteMonitoredItem
    void deleteMonitoredItem() {
        services::deleteMonitoredItem(connection(), subscriptionId(), monitoredItemId())
            .throwIfBad();
    }

private:
    Connection* connection_;
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
