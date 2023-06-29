#pragma once

#include <map>
#include <memory>

#include "open62541pp/services/NodeManagement.h"
#include "open62541pp/services/Subscription.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/NodeId.h"

namespace opcua {

/**
 * Internal storage for Server class.
 * Mainly used to store stateful function pointers.
 */
class ServerContext {
public:
    struct MonitoredItem {
        ReadValueId itemToMonitor;
        services::DataChangeNotificationCallback dataChangeCallback;
    };

    struct NodeContext {
        services::MethodCallback methodCallback;
    };

    std::map<uint32_t, std::unique_ptr<MonitoredItem>> monitoredItems;
    std::map<NodeId, std::unique_ptr<NodeContext>> nodeContexts;
};

}  // namespace opcua
