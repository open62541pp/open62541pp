#include "open62541pp/subscription.hpp"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include "open62541pp/client.hpp"
#include "open62541pp/detail/client_context.hpp"
#include "open62541pp/detail/server_context.hpp"
#include "open62541pp/server.hpp"

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
