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

/* ---------------------------------------------------------------------------------------------- */

inline void setContext(UA_Server* server, ServerContext& context) {
    assert(server != nullptr);  // NOLINT
    UA_Server_getConfig(server)->context = &context;
}

inline ServerContext& getContext(UA_Server* server) {
    assert(server != nullptr);  // NOLINT
    void* context = UA_Server_getConfig(server)->context;
    assert(context != nullptr);  // NOLINT
    return *static_cast<ServerContext*>(context);
}

}  // namespace opcua
