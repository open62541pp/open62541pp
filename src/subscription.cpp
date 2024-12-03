#include "open62541pp/subscription.hpp"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include "open62541pp/client.hpp"
#include "open62541pp/detail/client_context.hpp"
#include "open62541pp/detail/server_context.hpp"
#include "open62541pp/server.hpp"

namespace opcua {

template <typename T>
std::vector<MonitoredItem<T>> Subscription<T>::monitoredItems() {
    std::vector<MonitoredItem<T>> result;
    auto& monitoredItems = opcua::detail::getContext(connection()).monitoredItems;
    monitoredItems.eraseStale();
    monitoredItems.iterate([&](const auto& pair) {
        const auto [subId, monId] = pair.first;
        if (subId == subscriptionId()) {
            result.emplace_back(connection(), subId, monId);
        }
    });
    return result;
}

// explicit template instantiation
template std::vector<MonitoredItem<Client>> Subscription<Client>::monitoredItems();
template std::vector<MonitoredItem<Server>> Subscription<Server>::monitoredItems();

}  // namespace opcua

#endif
