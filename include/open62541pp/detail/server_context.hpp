#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include <utility>  // pair
#include <variant>

#include "open62541pp/config.hpp"
#include "open62541pp/detail/contextmap.hpp"
#include "open62541pp/detail/exceptioncatcher.hpp"
#include "open62541pp/detail/open62541/common.h"  // UA_AccessControl
#include "open62541pp/detail/ptr.hpp"
#include "open62541pp/plugin/nodestore.hpp"
#include "open62541pp/services/detail/monitoreditem_context.hpp"
#include "open62541pp/types.hpp"  // NodeId, Variant
#include "open62541pp/ua/types.hpp"  // IntegerId

namespace opcua {
class Session;
}  // namespace opcua

namespace opcua::detail {

struct NodeContext {
    UniqueOrRawPtr<ValueCallbackBase> valueCallback;
    UniqueOrRawPtr<DataSourceBase> dataSource;

#ifdef UA_ENABLE_METHODCALLS
    using MethodCallback = std::variant<
        std::function<void(Span<const Variant> input, Span<Variant> output)>,
        std::function<StatusCode(
            Session& session,
            Span<const Variant> input,
            Span<Variant> output,
            const NodeId& methodId,
            const NodeId& objectId
        )>>;

    MethodCallback methodCallback;
#endif
};

struct SessionRegistry {
    using Context = void*;

    decltype(UA_AccessControl::activateSession) activateSessionUser{nullptr};
    decltype(UA_AccessControl::closeSession) closeSessionUser{nullptr};
    std::map<NodeId, Context> sessions;
    std::mutex mutex;
};

/**
 * Internal storage for Server class.
 */
struct ServerContext {
    ExceptionCatcher exceptionCatcher;
    SessionRegistry sessionRegistry;
    std::atomic<bool> running{false};
    std::mutex mutexRun;

    ContextMap<uint64_t, Staleable<std::function<void()>>> callbacks;
    ContextMap<NodeId, NodeContext> nodeContexts;

#ifdef UA_ENABLE_SUBSCRIPTIONS
    using SubId = IntegerId;  // always 0
    using MonId = IntegerId;
    using SubMonId = std::pair<SubId, MonId>;
    ContextMap<SubMonId, services::detail::MonitoredItemContext> monitoredItems;
#endif
};

}  // namespace opcua::detail
