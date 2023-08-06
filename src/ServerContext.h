#pragma once

#include <map>
#include <memory>

#include "open62541pp/Config.h"
#include "open62541pp/ValueBackend.h"
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
#ifdef UA_ENABLE_SUBSCRIPTIONS
    struct MonitoredItem {
        ReadValueId itemToMonitor;
        services::DataChangeNotificationCallback dataChangeCallback;
    };

    std::map<uint32_t, std::unique_ptr<MonitoredItem>> monitoredItems;
#endif

    struct NodeContext {
        ValueCallback valueCallback;
        ValueBackendDataSource dataSource;
#ifdef UA_ENABLE_METHODCALLS
        services::MethodCallback methodCallback;
#endif
    };

    std::map<NodeId, std::unique_ptr<NodeContext>> nodeContexts;

    NodeContext* getOrCreateNodeContext(const NodeId& id) {
        return nodeContexts.emplace(id, std::make_unique<NodeContext>()).first->second.get();
    }
};

}  // namespace opcua
