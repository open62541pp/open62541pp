#include "open62541pp/Subscription.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include <atomic>
#include <utility>  // move

#include "open62541pp/Client.h"
#include "open62541pp/Server.h"
#include "open62541pp/Span.h"
#include "open62541pp/services/MonitoredItem.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/ExtensionObject.h"

#include "ClientContext.h"
#include "ServerContext.h"

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
    MonitoringParameters parameters;
    return subscribeDataChange(
        id, attribute, MonitoringMode::Reporting, parameters, std::move(onDataChange)
    );
}

/* ----------------------------------- Server specializations ----------------------------------- */

template <>
Subscription<Server>::Subscription(
    Server& connection,
    [[maybe_unused]] uint32_t subscriptionId  // ignore specified id and use default 0U
) noexcept
    : connection_(connection) {}

template <>
std::vector<MonitoredItem<Server>> Subscription<Server>::getMonitoredItems() {
    const auto& monitoredItems = connection_.getContext().monitoredItems;
    std::vector<MonitoredItem<Server>> result;
    result.reserve(monitoredItems.size());
    for (const auto& [monId, _] : monitoredItems) {
        result.emplace_back(connection_, 0U, monId);
    }
    return result;
}

template <>
MonitoredItem<Server> Subscription<Server>::subscribeDataChange(
    const NodeId& id,
    AttributeId attribute,
    MonitoringMode monitoringMode,
    MonitoringParameters& parameters,
    DataChangeCallback<Server> onDataChange
) {
    const uint32_t monitoredItemId = services::createMonitoredItemDataChange(
        connection_,
        {id, attribute},
        monitoringMode,
        parameters,
        [&, callback = std::move(onDataChange)](
            [[maybe_unused]] uint32_t subId, uint32_t monId, const DataValue& value
        ) {
            // workaround to prevent immediate callbacks, e.g. MonitoredItem::getNodeId() would
            // throw an exception (BadMonitoredItemidInvalid)
            // -> wait until monitoredItemContext is inserted in ServerContext::monitoredItems
            static std::atomic<bool> initialized = false;
            if (!initialized) {
                if (connection_.getContext().monitoredItems.count(monId) == 0) {
                    return;  // not initialized yet, skip
                }
                initialized = true;
            }

            static const MonitoredItem<Server> monitoredItem(connection_, 0U, monId);
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
std::vector<MonitoredItem<Client>> Subscription<Client>::getMonitoredItems() {
    const auto& monitoredItems = connection_.getContext().monitoredItems;
    std::vector<MonitoredItem<Client>> result;
    for (const auto& [subMonId, _] : monitoredItems) {
        const auto [subId, monId] = subMonId;
        if (subId == subscriptionId_) {
            result.emplace_back(connection_, subId, monId);
        }
    }
    return result;
}

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
    MonitoringParameters& parameters,
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
    MonitoringParameters& parameters,
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
    MonitoringParameters parameters;
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
