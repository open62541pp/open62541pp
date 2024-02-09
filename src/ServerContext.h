#pragma once

#include <map>
#include <memory>

#include "open62541pp/Config.h"
#include "open62541pp/ValueBackend.h"
#include "open62541pp/detail/ContextMap.h"
#include "open62541pp/detail/ExceptionCatcher.h"
#include "open62541pp/services/NodeManagement.h"
#include "open62541pp/services/Subscription.h"
#include "open62541pp/services/detail/MonitoredItemContext.h"
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
    using SubId = uint32_t;  // always 0
    using MonId = uint32_t;
    using SubMonId = std::pair<uint32_t, uint32_t>;

    detail::ContextMap<SubMonId, services::detail::MonitoredItemContext> monitoredItems;
#endif

    struct NodeContext {
        ValueCallback valueCallback;
        ValueBackendDataSource dataSource;
#ifdef UA_ENABLE_METHODCALLS
        services::MethodCallback methodCallback;
#endif
    };

    detail::ContextMap<NodeId, NodeContext> nodeContexts;

    detail::ExceptionCatcher exceptionCatcher;
};

}  // namespace opcua
