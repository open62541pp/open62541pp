#pragma once

#include <cstdint>
#include <set>

#include "open62541pp/Config.h"
#include "open62541pp/detail/ContextMap.h"
#include "open62541pp/detail/ExceptionCatcher.h"
#include "open62541pp/detail/NodeContext.h"
#include "open62541pp/detail/open62541/common.h"  // UA_AccessControl
#include "open62541pp/services/detail/MonitoredItemContext.h"
#include "open62541pp/types/NodeId.h"

namespace opcua::detail {

struct SessionRegistry {
    decltype(UA_AccessControl::activateSession) activateSessionUser{nullptr};
    decltype(UA_AccessControl::closeSession) closeSessionUser{nullptr};
    std::set<NodeId> sessionIds;
};

/**
 * Internal storage for Server class.
 * Mainly used to store stateful function pointers.
 */
struct ServerContext {
#ifdef UA_ENABLE_SUBSCRIPTIONS
    using SubId = uint32_t;  // always 0
    using MonId = uint32_t;
    using SubMonId = std::pair<uint32_t, uint32_t>;
    ContextMap<SubMonId, services::detail::MonitoredItemContext> monitoredItems;
#endif

    ContextMap<NodeId, NodeContext> nodeContexts;
    SessionRegistry sessionRegistry;
    ExceptionCatcher exceptionCatcher;
};

}  // namespace opcua::detail
