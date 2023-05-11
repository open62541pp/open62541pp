#include "open62541pp/Subscription.h"

#include <atomic>
#include <tuple>  // ignore
#include <utility>  // move

#include "open62541pp/Client.h"
#include "open62541pp/Server.h"
#include "open62541pp/services/MonitoredItem.h"

#include "ClientContext.h"
#include "ServerContext.h"

namespace opcua {

/* ------------------------------------------- Server ------------------------------------------- */

Subscription<Server>::Subscription(Server& server) noexcept
    : server_(server) {}

Server& Subscription<Server>::getConnection() noexcept {
    return server_;
}

const Server& Subscription<Server>::getConnection() const noexcept {
    return server_;
}

std::vector<MonitoredItem<Server>> Subscription<Server>::getMonitoredItems() {
    const auto& monitoredItems = server_.getContext().monitoredItems;
    std::vector<MonitoredItem<Server>> result;
    result.reserve(monitoredItems.size());
    for (const auto& [monId, _] : monitoredItems) {
        result.emplace_back(server_, monId);
    }
    return result;
}

MonitoredItem<Server> Subscription<Server>::subscribeDataChange(
    const NodeId& id,
    AttributeId attribute,
    MonitoringMode monitoringMode,
    MonitoringParameters& parameters,
    DataChangeCallback<Server> onDataChange
) {
    const uint32_t monitoredItemId = services::createMonitoredItemDataChange(
        server_,
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
                if (server_.getContext().monitoredItems.count(monId) == 0) {
                    return;  // not initialized yet, skip
                }
                initialized = true;
            }

            static const MonitoredItem<Server> monitoredItem(server_, monId);
            callback(monitoredItem, value);
        }
    );
    return {server_, monitoredItemId};
}

bool operator==(const Subscription<Server>& left, const Subscription<Server>& right) noexcept {
    return (left.getConnection() == right.getConnection());
}

bool operator!=(const Subscription<Server>& left, const Subscription<Server>& right) noexcept {
    return !(left == right);
}

/* ------------------------------------------- Client ------------------------------------------- */

Subscription<Client>::Subscription(Client& client, uint32_t subscriptionId) noexcept
    : client_(client),
      subscriptionId_(subscriptionId) {}

Client& Subscription<Client>::getConnection() noexcept {
    return client_;
}

const Client& Subscription<Client>::getConnection() const noexcept {
    return client_;
}

uint32_t Subscription<Client>::getSubscriptionId() const noexcept {
    return subscriptionId_;
}

std::vector<MonitoredItem<Client>> Subscription<Client>::getMonitoredItems() {
    const auto& monitoredItems = client_.getContext().monitoredItems;
    std::vector<MonitoredItem<Client>> result;
    for (const auto& [subMonId, _] : monitoredItems) {
        const auto [subId, monId] = subMonId;
        if (subId == subscriptionId_) {
            result.emplace_back(client_, subId, monId);
        }
    }
    return result;
}

void Subscription<Client>::setSubscriptionParameters(SubscriptionParameters& parameters) {
    services::modifySubscription(client_, subscriptionId_, parameters);
}

void Subscription<Client>::setPublishingMode(bool publishing) {
    services::setPublishingMode(client_, subscriptionId_, publishing);
}

MonitoredItem<Client> Subscription<Client>::subscribeDataChange(
    const NodeId& id,
    AttributeId attribute,
    MonitoringMode monitoringMode,
    MonitoringParameters& parameters,
    DataChangeCallback<Client> onDataChange
) {
    const uint32_t monitoredItemId = services::createMonitoredItemDataChange(
        client_,
        subscriptionId_,
        {id, attribute},
        monitoringMode,
        parameters,
        [&, callback = std::move(onDataChange)](
            uint32_t subId, uint32_t monId, const DataValue& value
        ) {
            static const MonitoredItem<Client> monitoredItem(client_, subId, monId);
            callback(monitoredItem, value);
        }
    );
    return {client_, subscriptionId_, monitoredItemId};
}

MonitoredItem<Client> Subscription<Client>::subscribeEvent(
    const NodeId& id,
    MonitoringMode monitoringMode,
    MonitoringParameters& parameters,
    EventCallback<Client> onEvent
) {
    const uint32_t monitoredItemId = services::createMonitoredItemEvent(
        client_,
        subscriptionId_,
        {id, AttributeId::EventNotifier},
        monitoringMode,
        parameters,
        [&, callback = std::move(onEvent)](
            uint32_t subId, uint32_t monId, const std::vector<Variant>& eventFields
        ) {
            static const MonitoredItem<Client> monitoredItem(client_, subId, monId);
            callback(monitoredItem, eventFields);
        }
    );
    return {client_, subscriptionId_, monitoredItemId};
}

void Subscription<Client>::deleteSubscription() {
    services::deleteSubscription(client_, subscriptionId_);
}

bool operator==(Subscription<Client>& left, Subscription<Client>& right) noexcept {
    return (left.getConnection() == right.getConnection()) &&
           (left.getSubscriptionId() == right.getSubscriptionId());
}

bool operator!=(Subscription<Client>& left, Subscription<Client>& right) noexcept {
    return !(left == right);
}

}  // namespace opcua
