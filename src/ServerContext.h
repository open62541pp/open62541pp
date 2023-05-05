#pragma once

#include <memory>
#include <utility>  // move
#include <vector>

#include "open62541pp/services/Subscription.h"

namespace opcua {

/**
 * Internal storage for Server class.
 * Mainly used to store stateful function pointers.
 */
class ServerContext {
public:
    struct MonitoredItem {
        services::DataChangeNotificationCallback dataChangeCallback;
        bool deleted = false;
    };

    void addSubscription(services::DeleteSubscriptionCallback deleteCallback) {
        deleteSubscriptionCallback_ = std::move(deleteCallback);
    }

    void deleteSubscription() {
        // TODO: delete all monitored items -> call delete callback and remove from container
        if (deleteSubscriptionCallback_) {
            deleteSubscriptionCallback_(0U);
            deleteSubscriptionCallback_ = nullptr;
        }
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
        removeIf(monitoredItems_, hasDeletedFlag);
    }

    services::DeleteSubscriptionCallback deleteSubscriptionCallback_;
    std::vector<std::unique_ptr<MonitoredItem>> monitoredItems_;
};

}  // namespace opcua
