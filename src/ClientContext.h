#pragma once

#include <array>
#include <cassert>
#include <map>
#include <memory>
#include <utility>  // pair

#include "open62541pp/Client.h"
#include "open62541pp/Config.h"
#include "open62541pp/services/MonitoredItem.h"
#include "open62541pp/services/Subscription.h"
#include "open62541pp/types/Composed.h"

#include "open62541_impl.h"

namespace opcua {

enum class ClientState {
    Disconnected,
    Connected,
    SessionActivated,
    SessionClosed,
};
inline constexpr size_t clientStateCount = 4;

/**
 * Internal storage for Client class.
 * Mainly used to store stateful function pointers.
 */
class ClientContext {
public:
#ifdef UA_ENABLE_SUBSCRIPTIONS
    struct Subscription {
        services::DeleteSubscriptionCallback deleteCallback;
    };

    struct MonitoredItem {
        ReadValueId itemToMonitor;
        services::DataChangeNotificationCallback dataChangeCallback;
        services::EventNotificationCallback eventCallback;
        services::DeleteMonitoredItemCallback deleteCallback;
    };

    using SubId = uint32_t;
    using MonId = uint32_t;
    using SubMonId = std::pair<uint32_t, uint32_t>;

    std::map<SubId, std::unique_ptr<Subscription>> subscriptions;
    std::map<SubMonId, std::unique_ptr<MonitoredItem>> monitoredItems;
#endif

#if UAPP_OPEN62541_VER_LE(1, 0)
    UA_ClientState lastClientState{};
#else
    UA_SecureChannelState lastChannelState{};
    UA_SessionState lastSessionState{};
#endif
    std::array<StateCallback, clientStateCount> stateCallbacks;
};

/* ---------------------------------------------------------------------------------------------- */

inline ClientContext& getContext(UA_Client* client) {
    assert(client != nullptr);
    void* context = UA_Client_getConfig(client)->clientContext;
    assert(context != nullptr);
    return *static_cast<ClientContext*>(context);
}

}  // namespace opcua
