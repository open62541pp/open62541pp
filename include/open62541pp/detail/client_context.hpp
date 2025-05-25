#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <utility>  // pair

#include "open62541pp/config.hpp"
#include "open62541pp/detail/contextmap.hpp"
#include "open62541pp/detail/exceptioncatcher.hpp"
#include "open62541pp/detail/open62541/client.h"  // UA_SessionState, UA_SecureChannelState
#include "open62541pp/services/detail/monitoreditem_context.hpp"
#include "open62541pp/services/detail/subscription_context.hpp"
#include "open62541pp/ua/types.hpp"  // IntegerId

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
 */
struct ClientContext {
    ExceptionCatcher exceptionCatcher;
    std::atomic<bool> running{false};

#if UAPP_OPEN62541_VER_LE(1, 0)
    UA_ClientState lastClientState{};
#else
    UA_SessionState lastSessionState{};
    UA_SecureChannelState lastChannelState{};
#endif
    std::array<std::function<void()>, clientStateCount> stateCallbacks;
    std::function<void()> inactivityCallback;
    ContextMap<uint64_t, Staleable<std::function<void()>>> callbacks;

#ifdef UA_ENABLE_SUBSCRIPTIONS
    using SubId = IntegerId;
    using MonId = IntegerId;
    using SubMonId = std::pair<SubId, MonId>;
    ContextMap<SubId, services::detail::SubscriptionContext> subscriptions;
    ContextMap<SubMonId, services::detail::MonitoredItemContext> monitoredItems;
    std::function<void(IntegerId)> subscriptionInactivityCallback;
#endif
};

}  // namespace opcua::detail
