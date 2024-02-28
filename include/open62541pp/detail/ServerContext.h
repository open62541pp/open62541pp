#pragma once

#include <cstdint>

#include "open62541pp/Config.h"
#include "open62541pp/detail/ContextMap.h"
#include "open62541pp/detail/ExceptionCatcher.h"
#include "open62541pp/detail/NodeContext.h"
#include "open62541pp/services/detail/MonitoredItemContext.h"
#include "open62541pp/types/NodeId.h"

namespace opcua::detail {

/**
 * Internal storage for Server class.
 * Mainly used to store stateful function pointers.
 */
struct ServerContext {
#ifdef UA_ENABLE_SUBSCRIPTIONS
    using SubId = uint32_t;  // always 0
    using MonId = uint32_t;
    using SubMonId = std::pair<uint32_t, uint32_t>;
    detail::ContextMap<SubMonId, services::detail::MonitoredItemContext> monitoredItems;
#endif

    detail::ContextMap<NodeId, NodeContext> nodeContexts;

    detail::ExceptionCatcher exceptionCatcher;
};

}  // namespace opcua::detail
