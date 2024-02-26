#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <utility>  // pair

#include "open62541pp/Client.h"
#include "open62541pp/Config.h"
#include "open62541pp/detail/ContextMap.h"
#include "open62541pp/detail/ExceptionCatcher.h"
#include "open62541pp/open62541.h"
#include "open62541pp/services/detail/MonitoredItemContext.h"
#include "open62541pp/services/detail/SubscriptionContext.h"

namespace opcua::detail {

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
    using SubId = uint32_t;
    using MonId = uint32_t;
    using SubMonId = std::pair<uint32_t, uint32_t>;
    detail::ContextMap<SubId, services::detail::SubscriptionContext> subscriptions;
    detail::ContextMap<SubMonId, services::detail::MonitoredItemContext> monitoredItems;
#endif

#if UAPP_OPEN62541_VER_LE(1, 0)
    UA_ClientState lastClientState{};
#else
    UA_SecureChannelState lastChannelState{};
    UA_SessionState lastSessionState{};
#endif
    std::array<StateCallback, clientStateCount> stateCallbacks;

    detail::ExceptionCatcher exceptionCatcher;
};

}  // namespace opcua::detail
