#include "open62541pp/Subscription.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include <atomic>
#include <utility>  // move

#include "open62541pp/Client.h"
#include "open62541pp/Server.h"
#include "open62541pp/Span.h"
#include "open62541pp/detail/ClientContext.h"
#include "open62541pp/detail/ServerContext.h"
#include "open62541pp/services/MonitoredItem.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/ExtensionObject.h"

namespace opcua {

template <typename T>
T& Subscription<T>::getConnection() noexcept {
    return connection_;
}

template <typename T>
const T& Subscription<T>::getConnection() const noexcept {
    return connection_;
}

template <typename T>
uint32_t Subscription<T>::getSubscriptionId() const noexcept {
    return subscriptionId_;
}

template <typename T>
MonitoredItem<T> Subscription<T>::subscribeDataChange(
    const NodeId& id, AttributeId attribute, DataChangeCallback<T> onDataChange
) {
    MonitoringParametersEx parameters;
    return subscribeDataChange(
        id, attribute, MonitoringMode::Reporting, parameters, std::move(onDataChange)
    );
}

template <typename T>
std::vector<MonitoredItem<T>> Subscription<T>::getMonitoredItems() {
    auto& monitoredItems = opcua::detail::getContext(connection_).monitoredItems;
    monitoredItems.eraseStale();
    auto lock = monitoredItems.acquireLock();
    const auto& map = monitoredItems.underlying();
    std::vector<MonitoredItem<T>> result;
    result.reserve(map.size());
    for (const auto& [subMonId, _] : map) {
        const auto [subId, monId] = subMonId;
        if (subId == subscriptionId_) {
            result.emplace_back(connection_, subId, monId);
        }
    }
    return result;
}

/* ----------------------------------- Server specializations ----------------------------------- */

template <>
Subscription<Server>::Subscription(
    Server& connection,
    [[maybe_unused]] uint32_t subscriptionId  // ignore specified id and use default 0U
) noexcept
    : connection_(connection) {}

template <>
MonitoredItem<Server> Subscription<Server>::subscribeDataChange(
    const NodeId& id,
    AttributeId attribute,
    MonitoringMode monitoringMode,
    MonitoringParametersEx& parameters,
    DataChangeCallback<Server> onDataChange
) {
    const uint32_t monitoredItemId = services::createMonitoredItemDataChange(
        connection_,
        {id, attribute},
        monitoringMode,
        parameters,
        [connectionPtr = &connection_, callback = std::move(onDataChange)](
            uint32_t subId, uint32_t monId, const DataValue& value
        ) {
            const MonitoredItem<Server> monitoredItem(*connectionPtr, subId, monId);
            callback(monitoredItem, value);
        }
    );
    return {connection_, 0U, monitoredItemId};
}

/* ----------------------------------- Client specializations ----------------------------------- */

template <>
Subscription<Client>::Subscription(Client& connection, uint32_t subscriptionId) noexcept
    : connection_(connection),
      subscriptionId_(subscriptionId) {}

template <>
void Subscription<Client>::setSubscriptionParameters(SubscriptionParameters& parameters) {
    services::modifySubscription(connection_, subscriptionId_, parameters);
}

template <>
void Subscription<Client>::setPublishingMode(bool publishing) {
    services::setPublishingMode(connection_, subscriptionId_, publishing);
}

template <>
MonitoredItem<Client> Subscription<Client>::subscribeDataChange(
    const NodeId& id,
    AttributeId attribute,
    MonitoringMode monitoringMode,
    MonitoringParametersEx& parameters,
    DataChangeCallback<Client> onDataChange
) {
    const uint32_t monitoredItemId = services::createMonitoredItemDataChange(
        connection_,
        subscriptionId_,
        {id, attribute},
        monitoringMode,
        parameters,
        [connectionPtr = &connection_, callback = std::move(onDataChange)](
            uint32_t subId, uint32_t monId, const DataValue& value
        ) {
            const MonitoredItem<Client> monitoredItem(*connectionPtr, subId, monId);
            callback(monitoredItem, value);
        }
    );
    return {connection_, subscriptionId_, monitoredItemId};
}

template <>
MonitoredItem<Client> Subscription<Client>::subscribeEvent(
    const NodeId& id,
    MonitoringMode monitoringMode,
    MonitoringParametersEx& parameters,
    EventCallback<Client> onEvent
) {
    const uint32_t monitoredItemId = services::createMonitoredItemEvent(
        connection_,
        subscriptionId_,
        {id, AttributeId::EventNotifier},
        monitoringMode,
        parameters,
        [connectionPtr = &connection_, callback = std::move(onEvent)](
            uint32_t subId, uint32_t monId, Span<const Variant> eventFields
        ) {
            const MonitoredItem<Client> monitoredItem(*connectionPtr, subId, monId);
            callback(monitoredItem, eventFields);
        }
    );
    return {connection_, subscriptionId_, monitoredItemId};
}

template <>
MonitoredItem<Client> Subscription<Client>::subscribeEvent(
    const NodeId& id, const EventFilter& eventFilter, EventCallback<Client> onEvent
) {
    MonitoringParametersEx parameters;
    parameters.filter = ExtensionObject::fromDecodedCopy(eventFilter);
    return subscribeEvent(id, MonitoringMode::Reporting, parameters, std::move(onEvent));
}

template <>
void Subscription<Client>::deleteSubscription() {
    services::deleteSubscription(connection_, subscriptionId_);
}

/* ---------------------------------------------------------------------------------------------- */

// explicit template instantiation
template class Subscription<Server>;
template class Subscription<Client>;

}  // namespace opcua

#endif
