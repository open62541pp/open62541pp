#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include <utility>  // exchange, pair
#include <variant>
#include <vector>

#include "open62541pp/config.hpp"
#include "open62541pp/detail/contextmap.hpp"
#include "open62541pp/detail/exceptioncatcher.hpp"
#include "open62541pp/detail/open62541/common.h"  // UA_AccessControl
#include "open62541pp/detail/open62541/server.h"  // UA_HistoryDataGathering
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

#if UAPP_HAS_HISTORIZING
/**
 * Owner of a history data backend.
 * The default history data gathering doesn't free the backends of the registered nodes, so the
 * server has to keep and free them.
 */
class HistoryDataBackendMemory {
public:
    HistoryDataBackendMemory() = default;

    explicit HistoryDataBackendMemory(UA_HistoryDataBackend backend) noexcept
        : backend_{backend} {}

    ~HistoryDataBackendMemory() {
        clear();
    }

    HistoryDataBackendMemory(const HistoryDataBackendMemory&) = delete;
    HistoryDataBackendMemory& operator=(const HistoryDataBackendMemory&) = delete;

    HistoryDataBackendMemory(HistoryDataBackendMemory&& other) noexcept
        : backend_{std::exchange(other.backend_, {})} {}

    HistoryDataBackendMemory& operator=(HistoryDataBackendMemory&& other) noexcept {
        if (this != &other) {
            clear();
            backend_ = std::exchange(other.backend_, {});
        }
        return *this;
    }

private:
    void clear() noexcept {
        if (backend_.context != nullptr) {
            UA_HistoryDataBackend_Memory_clear(&backend_);
        }
    }

    UA_HistoryDataBackend backend_{};
};
#endif

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
    std::mutex mutexStartup;
    bool stopRequested{false};  // guarded by mutexStartup

    ContextMap<uint64_t, Staleable<std::function<void()>>> callbacks;
    ContextMap<NodeId, NodeContext> nodeContexts;

#if UAPP_HAS_HISTORIZING
    // gathering of the default history database, needed to historize nodes
    UA_HistoryDataGathering historyDataGathering{};
    // backends of the historized nodes, freed with the server
    std::vector<HistoryDataBackendMemory> historyDataBackends;
#endif

#ifdef UA_ENABLE_SUBSCRIPTIONS
    using SubId = IntegerId;  // always 0
    using MonId = IntegerId;
    using SubMonId = std::pair<SubId, MonId>;
    ContextMap<SubMonId, services::detail::MonitoredItemContext> monitoredItems;
#endif
};

}  // namespace opcua::detail
