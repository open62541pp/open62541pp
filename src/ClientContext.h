#pragma once

#include <algorithm>  // remove_if
#include <memory>
#include <vector>

#include "open62541pp/services/MonitoredItem.h"
#include "open62541pp/services/Subscription.h"

namespace opcua {

/**
 * Internal storage for Client class.
 * Mainly used to store stateful function pointers.
 */
class ClientContext {
public:
    struct Subscription {
        services::DeleteSubscriptionCallback deleteCallback;
        bool deleted = false;
    };

    struct MonitoredItem {
        services::DataChangeNotificationCallback dataChangeCallback;
        services::EventNotificationCallback eventCallback;
        services::DeleteMonitoredItemCallback deleteCallback;
        bool deleted = false;
    };

    void addSubscription(std::unique_ptr<Subscription>&& subscription) {
        cleanDeleted();
        subscriptions_.push_back(std::move(subscription));
    }

    void addMonitoredItem(std::unique_ptr<MonitoredItem>&& monitoredItem) {
        cleanDeleted();
        monitoredItems_.push_back(std::move(monitoredItem));
    }

private:
    void cleanDeleted() {
        const auto hasDeletedFlag = [](auto&& e) { return e->deleted; };
        const auto removeIf = [](auto& c, auto pred) {
            c.erase(std::remove_if(c.begin(), c.end(), pred), c.end());
        };
        removeIf(subscriptions_, hasDeletedFlag);
        removeIf(monitoredItems_, hasDeletedFlag);
    }

    std::vector<std::unique_ptr<Subscription>> subscriptions_;
    std::vector<std::unique_ptr<MonitoredItem>> monitoredItems_;
};

}  // namespace opcua
