#pragma once

#include <map>

#include "open62541pp/services/Subscription.h"

namespace opcua {

/**
 * Internal storage for Client class.
 * Mainly used to store stateful function pointers.
 */
struct ClientContext {
    struct SubscriptionContext {
        services::DeleteSubscriptionCallback deleteSubscriptionCallback;
    };

    std::map<uint32_t, SubscriptionContext> subscriptions;
};

}  // namespace opcua
