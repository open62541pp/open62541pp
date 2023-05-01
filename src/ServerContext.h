#pragma once

#include <map>

#include "open62541pp/services/Subscription.h"

namespace opcua {

/**
 * Internal storage for Server class.
 * Mainly used to store stateful function pointers.
 */
struct ServerContext {
    services::DeleteSubscriptionCallback deleteSubscriptionCallback;
};

}  // namespace opcua
