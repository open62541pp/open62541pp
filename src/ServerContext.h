#pragma once

#include <cassert>
#include <map>
#include <memory>

#include "open62541pp/services/Subscription.h"

#include "open62541_impl.h"

namespace opcua {

/**
 * Internal storage for Server class.
 * Mainly used to store stateful function pointers.
 */
class ServerContext {
public:
    struct MonitoredItem {
        services::DataChangeNotificationCallback dataChangeCallback;
    };

    std::map<uint32_t, std::unique_ptr<MonitoredItem>> monitoredItems;
};

}  // namespace opcua
