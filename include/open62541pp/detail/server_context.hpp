#pragma once

#include <cstdint>
#include <functional>
#include <set>

#include "open62541pp/config.hpp"
#include "open62541pp/detail/contextmap.hpp"
#include "open62541pp/detail/exceptioncatcher.hpp"
#include "open62541pp/detail/open62541/common.h"  // UA_AccessControl
#include "open62541pp/plugins/nodestore.hpp"
#include "open62541pp/services/detail/monitoreditem_context.hpp"
#include "open62541pp/types.hpp"

namespace opcua::detail {

struct NodeContext {
    ValueCallback valueCallback;
    ValueBackendDataSource dataSource;
#ifdef UA_ENABLE_METHODCALLS
    std::function<void(Span<const Variant> input, Span<Variant> output)> methodCallback;
#endif
};

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
