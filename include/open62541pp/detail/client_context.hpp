#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <utility>  // pair

#include "open62541pp/client.hpp"
#include "open62541pp/config.hpp"
#include "open62541pp/detail/contextmap.hpp"
#include "open62541pp/detail/exceptioncatcher.hpp"
#include "open62541pp/services/detail/monitoreditem_context.hpp"
#include "open62541pp/services/detail/subscription_context.hpp"

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
struct ClientContext {
#ifdef UA_ENABLE_SUBSCRIPTIONS
    using SubId = uint32_t;
    using MonId = uint32_t;
    using SubMonId = std::pair<uint32_t, uint32_t>;
    ContextMap<SubId, services::detail::SubscriptionContext> subscriptions;
    ContextMap<SubMonId, services::detail::MonitoredItemContext> monitoredItems;
#endif

#if UAPP_OPEN62541_VER_LE(1, 0)
    UA_ClientState lastClientState{};
#else
    UA_SecureChannelState lastChannelState{};
    UA_SessionState lastSessionState{};
#endif
    std::array<StateCallback, clientStateCount> stateCallbacks;
    InactivityCallback inactivityCallback;

    ExceptionCatcher exceptionCatcher;
};

}  // namespace opcua::detail
