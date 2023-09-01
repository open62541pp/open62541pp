#include "open62541pp/MonitoredItem.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include <cassert>

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/Subscription.h"
#include "open62541pp/open62541.h"

#include "ClientContext.h"
#include "ServerContext.h"

namespace opcua {

template <typename T>
MonitoredItem<T>::MonitoredItem(
    T& connection, uint32_t subscriptionId, uint32_t monitoredItemId
) noexcept
    : connection_(connection),
      subscriptionId_(subscriptionId),
      monitoredItemId_(monitoredItemId) {}

template <typename T>
T& MonitoredItem<T>::getConnection() noexcept {
    return connection_;
}

template <typename T>
const T& MonitoredItem<T>::getConnection() const noexcept {
    return connection_;
}

template <typename T>
uint32_t MonitoredItem<T>::getSubscriptionId() const noexcept {
    return subscriptionId_;
}

template <typename T>
uint32_t MonitoredItem<T>::getMonitoredItemId() const noexcept {
    return monitoredItemId_;
}

template <typename T>
Subscription<T> MonitoredItem<T>::getSubscription() const {
    return {connection_, subscriptionId_};
}

inline static ServerContext::MonitoredItem& getMonitoredItemContext(
    Server& server, [[maybe_unused]] uint32_t subscriptionId, uint32_t monitoredItemId
) {
    auto& monitoredItems = server.getContext().monitoredItems;
    auto it = monitoredItems.find(monitoredItemId);
    if (it == monitoredItems.end()) {
        throw BadStatus(UA_STATUSCODE_BADMONITOREDITEMIDINVALID);
    }
    assert(it->second != nullptr);
    return *(it->second);
}

inline static ClientContext::MonitoredItem& getMonitoredItemContext(
    Client& client, uint32_t subscriptionId, uint32_t monitoredItemId
) {
    auto& monitoredItems = client.getContext().monitoredItems;
    auto it = monitoredItems.find({subscriptionId, monitoredItemId});
    if (it == monitoredItems.end()) {
        throw BadStatus(UA_STATUSCODE_BADMONITOREDITEMIDINVALID);
    }
    assert(it->second != nullptr);
    return *(it->second);
}

template <typename T>
const NodeId& MonitoredItem<T>::getNodeId() const {
    return getMonitoredItemContext(connection_, subscriptionId_, monitoredItemId_)
        .itemToMonitor.getNodeId();
}

template <typename T>
AttributeId MonitoredItem<T>::getAttributeId() const {
    return getMonitoredItemContext(connection_, subscriptionId_, monitoredItemId_)
        .itemToMonitor.getAttributeId();
}

/* ----------------------------------- Server specializations ----------------------------------- */

template <>
MonitoredItem<Server>::MonitoredItem(
    Server& connection, [[maybe_unused]] uint32_t subscriptionId, uint32_t monitoredItemId
) noexcept
    : connection_(connection),
      monitoredItemId_(monitoredItemId) {}

template <>
void MonitoredItem<Server>::deleteMonitoredItem() {
    services::deleteMonitoredItem(connection_, monitoredItemId_);
}

/* ----------------------------------- Client specializations ----------------------------------- */

template <>
void MonitoredItem<Client>::setMonitoringParameters(MonitoringParameters& parameters) {
    services::modifyMonitoredItem(connection_, subscriptionId_, monitoredItemId_, parameters);
}

template <>
void MonitoredItem<Client>::setMonitoringMode(MonitoringMode monitoringMode) {
    services::setMonitoringMode(connection_, subscriptionId_, monitoredItemId_, monitoringMode);
}

template <>
void MonitoredItem<Client>::deleteMonitoredItem() {
    services::deleteMonitoredItem(connection_, subscriptionId_, monitoredItemId_);
}

/* ---------------------------------------------------------------------------------------------- */

// explicit template instantiation
template class MonitoredItem<Server>;
template class MonitoredItem<Client>;

}  // namespace opcua

#endif
