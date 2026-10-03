#include "open62541pp/plugin/historydatabase.hpp"

#if UAPP_HAS_HISTORIZING

#include "open62541pp/detail/open62541/server.h"
#include "open62541pp/detail/server_context.hpp"
#include "open62541pp/server.hpp"
#include "open62541pp/types.hpp"

namespace opcua {

void useHistoryDatabase(Server& server, size_t capacity) {
    auto& gathering = detail::getContext(server).historyDataGathering;
    if (gathering.context != nullptr) {
        return;  // already enabled
    }
    gathering = UA_HistoryDataGathering_Default(capacity);
    // the history database takes ownership of the gathering and frees it with the server config
    server.config().handle()->historyDatabase = UA_HistoryDatabase_default(gathering);
}

StatusCode historizeNode(Server& server, const NodeId& id, const HistorizingSettings& settings) {
    auto& context = detail::getContext(server);
    auto& gathering = context.historyDataGathering;
    if (gathering.registerNodeId == nullptr) {
        return UA_STATUSCODE_BADCONFIGURATIONERROR;  // useHistoryDatabase not called
    }

    UA_HistorizingNodeIdSettings setting{};
    setting.historizingBackend = UA_HistoryDataBackend_Memory(1, settings.capacity);
    setting.maxHistoryDataResponseSize = settings.maxResponseSize;
    setting.historizingUpdateStrategy = static_cast<UA_HistorizingUpdateStrategy>(
        settings.updateStrategy
    );
    setting.pollingInterval = settings.pollingInterval;

    // not const, so the status can be moved out of the function
    StatusCode status = gathering.registerNodeId(
        server.handle(), gathering.context, id.handle(), setting
    );
    if (status.isBad()) {
        UA_HistoryDataBackend_Memory_clear(&setting.historizingBackend);
        return status;
    }
    // the gathering doesn't free the backends of the registered nodes, the server owns them
    context.historyDataBackends.emplace_back(setting.historizingBackend);
    return UA_Server_writeHistorizing(server.handle(), id, true);
}

}  // namespace opcua

#endif
