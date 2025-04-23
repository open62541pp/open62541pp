#include "open62541pp/subscription.hpp"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include "open62541pp/client.hpp"
#include "open62541pp/detail/client_context.hpp"
#include "open62541pp/detail/server_context.hpp"
#include "open62541pp/server.hpp"

namespace opcua {

static auto createSubscription(Client& connection, const SubscriptionParameters& parameters) {
    const auto response = services::createSubscription(connection, parameters, true, {}, {});
    response.responseHeader().serviceResult().throwIfBad();
    return response.subscriptionId();
}

template <>
Subscription<Client>::Subscription(Client& connection, const SubscriptionParameters& parameters)
    : Subscription{connection, createSubscription(connection, parameters)} {}

template <>
Subscription<Server>::Subscription(
    Server& connection, [[maybe_unused]] const SubscriptionParameters& parameters
)
    : Subscription{connection, 0U} {}

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
