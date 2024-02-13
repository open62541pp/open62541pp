#pragma once

#include <cstdint>
#include <functional>

#include "open62541pp/detail/Staleable.h"
#include "open62541pp/services/detail/CallbackAdapter.h"

// forward declare
struct UA_Client;

namespace opcua::services::detail {

struct SubscriptionContext : CallbackAdapter, opcua::detail::Staleable {
    std::function<void(uint32_t subId)> deleteCallback;

    static void deleteCallbackNative(
        [[maybe_unused]] UA_Client* client, uint32_t subId, void* subContext
    ) noexcept {
        if (subContext != nullptr) {
            auto* self = static_cast<SubscriptionContext*>(subContext);
            self->invoke(self->deleteCallback, subId);
            self->stale = true;
        }
    }
};

}  // namespace opcua::services::detail
