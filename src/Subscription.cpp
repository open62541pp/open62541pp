#include "open62541pp/Subscription.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include "open62541pp/Client.h"
#include "open62541pp/Server.h"
#include "open62541pp/detail/ClientContext.h"
#include "open62541pp/detail/ServerContext.h"

namespace opcua {

template <typename T>
std::vector<MonitoredItem<T>> Subscription<T>::getMonitoredItems() {
    auto& monitoredItems = opcua::detail::getContext(connection()).monitoredItems;
    monitoredItems.eraseStale();
    auto lock = monitoredItems.acquireLock();
    const auto& map = monitoredItems.underlying();
    std::vector<MonitoredItem<T>> result;
    result.reserve(map.size());
    for (const auto& [subMonId, _] : map) {
        const auto [subId, monId] = subMonId;
        if (subId == subscriptionId()) {
            result.emplace_back(connection(), subId, monId);
        }
    }
    return result;
}

// explicit template instantiation
template std::vector<MonitoredItem<Client>> Subscription<Client>::getMonitoredItems();
template std::vector<MonitoredItem<Server>> Subscription<Server>::getMonitoredItems();

}  // namespace opcua

#endif
