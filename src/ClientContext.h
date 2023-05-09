#pragma once

#include <cassert>
#include <map>
#include <memory>
#include <utility>  // pair

#include "open62541pp/services/MonitoredItem.h"
#include "open62541pp/services/Subscription.h"
#include "open62541pp/types/Composed.h"

#include "open62541_impl.h"

namespace opcua {

/**
 * Internal storage for Client class.
 * Mainly used to store stateful function pointers.
 */
class ClientContext {
public:
    struct Subscription {
        services::DeleteSubscriptionCallback deleteCallback;
    };

    struct MonitoredItem {
        ReadValueId itemToMonitor;
        services::DataChangeNotificationCallback dataChangeCallback;
        services::EventNotificationCallback eventCallback;
        services::DeleteMonitoredItemCallback deleteCallback;
    };

    using SubId = uint32_t;
    using MonId = uint32_t;
    using SubMonId = std::pair<uint32_t, uint32_t>;

    std::map<SubId, std::unique_ptr<Subscription>> subscriptions;
    std::map<SubMonId, std::unique_ptr<MonitoredItem>> monitoredItems;
};

/* ---------------------------------------------------------------------------------------------- */

inline void setContext(UA_Client* client, ClientContext& context) {
    assert(client != nullptr);  // NOLINT
    UA_Client_getConfig(client)->clientContext = &context;
}

inline ClientContext& getContext(UA_Client* client) {
    assert(client != nullptr);  // NOLINT
    void* context = UA_Client_getConfig(client)->clientContext;
    assert(context != nullptr);  // NOLINT
    return *static_cast<ClientContext*>(context);
}

}  // namespace opcua
