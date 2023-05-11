#include "open62541pp/MonitoredItem.h"

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"

#include "ClientContext.h"
#include "ServerContext.h"

namespace opcua {

/* ------------------------------------------- Server ------------------------------------------- */

inline static ServerContext::MonitoredItem& getMonitoredItemContext(
    Server& server, uint32_t monitoredItemId
) {
    auto& monitoredItems = server.getContext().monitoredItems;
    auto it = monitoredItems.find(monitoredItemId);
    if (it == monitoredItems.end()) {
        throw BadStatus(UA_STATUSCODE_BADMONITOREDITEMIDINVALID);
    }
    return *(it->second);
}

MonitoredItem<Server>::MonitoredItem(Server& server, uint32_t monitoredItemId) noexcept
    : server_(server),
      monitoredItemId_(monitoredItemId) {}

Server& MonitoredItem<Server>::getConnection() noexcept {
    return server_;
}

const Server& MonitoredItem<Server>::getConnection() const noexcept {
    return server_;
}

uint32_t MonitoredItem<Server>::getMonitoredItemId() const noexcept {
    return monitoredItemId_;
}

Subscription<Server> MonitoredItem<Server>::getSubscription() const {
    return {server_, 0U};
}

const NodeId& MonitoredItem<Server>::getNodeId() const {
    return getMonitoredItemContext(server_, monitoredItemId_).itemToMonitor.getNodeId();
}

AttributeId MonitoredItem<Server>::getAttributeId() const {
    return getMonitoredItemContext(server_, monitoredItemId_).itemToMonitor.getAttributeId();
}

void MonitoredItem<Server>::deleteMonitoredItem() {
    services::deleteMonitoredItem(server_, monitoredItemId_);
}

bool operator==(const MonitoredItem<Server>& left, const MonitoredItem<Server>& right) noexcept {
    return (left.getConnection() == right.getConnection()) &&
           (left.getMonitoredItemId() == right.getMonitoredItemId());
}

bool operator!=(const MonitoredItem<Server>& left, const MonitoredItem<Server>& right) noexcept {
    return !(left == right);
}

/* ------------------------------------------- Client ------------------------------------------- */

inline static ClientContext::MonitoredItem& getMonitoredItemContext(
    Client& client, uint32_t subscriptionId, uint32_t monitoredItemId
) {
    auto& monitoredItems = client.getContext().monitoredItems;
    auto it = monitoredItems.find({subscriptionId, monitoredItemId});
    if (it == monitoredItems.end()) {
        throw BadStatus(UA_STATUSCODE_BADMONITOREDITEMIDINVALID);
    }
    return *(it->second);
}

MonitoredItem<Client>::MonitoredItem(
    Client& client, uint32_t subscriptionId, uint32_t monitoredItemId
) noexcept
    : client_(client),
      subscriptionId_(subscriptionId),
      monitoredItemId_(monitoredItemId) {}

Client& MonitoredItem<Client>::getConnection() noexcept {
    return client_;
}

const Client& MonitoredItem<Client>::getConnection() const noexcept {
    return client_;
}

uint32_t MonitoredItem<Client>::getSubscriptionId() const noexcept {
    return subscriptionId_;
}

uint32_t MonitoredItem<Client>::getMonitoredItemId() const noexcept {
    return monitoredItemId_;
}

Subscription<Client> MonitoredItem<Client>::getSubscription() const {
    return {client_, subscriptionId_};
}

const NodeId& MonitoredItem<Client>::getNodeId() const {
    return getMonitoredItemContext(client_, subscriptionId_, monitoredItemId_)
        .itemToMonitor.getNodeId();
}

AttributeId MonitoredItem<Client>::getAttributeId() const {
    return getMonitoredItemContext(client_, subscriptionId_, monitoredItemId_)
        .itemToMonitor.getAttributeId();
}

void MonitoredItem<Client>::setMonitoringParameters(MonitoringParameters& parameters) {
    services::modifyMonitoredItem(client_, subscriptionId_, monitoredItemId_, parameters);
}

void MonitoredItem<Client>::setMonitoringMode(MonitoringMode monitoringMode) {
    services::setMonitoringMode(client_, subscriptionId_, monitoredItemId_, monitoringMode);
}

void MonitoredItem<Client>::deleteMonitoredItem() {
    services::deleteMonitoredItem(client_, subscriptionId_, monitoredItemId_);
}

bool operator==(const MonitoredItem<Client>& left, const MonitoredItem<Client>& right) noexcept {
    return (left.getConnection() == right.getConnection()) &&
           (left.getSubscriptionId() == right.getSubscriptionId()) &&
           (left.getMonitoredItemId() == right.getMonitoredItemId());
}

bool operator!=(const MonitoredItem<Client>& left, const MonitoredItem<Client>& right) noexcept {
    return !(left == right);
}

}  // namespace opcua
