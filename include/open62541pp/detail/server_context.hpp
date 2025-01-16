#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <utility>  // pair
#include <vector>

#include "open62541pp/config.hpp"
#include "open62541pp/datatype.hpp"
#include "open62541pp/detail/contextmap.hpp"
#include "open62541pp/detail/exceptioncatcher.hpp"
#include "open62541pp/detail/open62541/common.h"  // UA_AccessControl
#include "open62541pp/plugin/nodestore.hpp"
#include "open62541pp/services/detail/monitoreditem_context.hpp"
#include "open62541pp/types.hpp"  // NodeId, Variant
#include "open62541pp/ua/types.hpp"  // IntegerId
#include "open62541pp/services/nodemanagement.hpp"

namespace opcua::detail {

struct NodeContext {
    ValueCallbackBase* valueCallback{nullptr};
    ValueBackendDataSource dataSource;
#ifdef UA_ENABLE_METHODCALLS
    opcua::services::MethodCallback methodCallback;
#endif
};

struct SessionRegistry {
    decltype(UA_AccessControl::activateSession) activateSessionUser{nullptr};
    decltype(UA_AccessControl::closeSession) closeSessionUser{nullptr};
    std::set<NodeId> sessionIds;
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

    std::vector<DataType> dataTypes;
    std::unique_ptr<UA_DataTypeArray> dataTypeArray;

#ifdef UA_ENABLE_SUBSCRIPTIONS
    using SubId = IntegerId;  // always 0
    using MonId = IntegerId;
    using SubMonId = std::pair<SubId, MonId>;
    ContextMap<SubMonId, services::detail::MonitoredItemContext> monitoredItems;
#endif

    ContextMap<NodeId, NodeContext> nodeContexts;
};

}  // namespace opcua::detail
