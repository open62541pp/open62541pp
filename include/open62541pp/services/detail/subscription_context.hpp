#pragma once

#include <functional>

#include "open62541pp/config.hpp"
#include "open62541pp/services/detail/callbackadapter.hpp"
#include "open62541pp/types_composed.hpp"  // IntegerId, StatusChangeNotification
#include "open62541pp/wrapper.hpp"  // asWrapper

#ifdef UA_ENABLE_SUBSCRIPTIONS

struct UA_Client;

namespace opcua::services::detail {

struct SubscriptionContext : CallbackAdapter {
    bool stale{false};
    std::function<void(IntegerId subId, StatusChangeNotification&)> statusChangeCallback;
    std::function<void(IntegerId subId)> deleteCallback;

    static void statusChangeCallbackNative(
        [[maybe_unused]] UA_Client* client,
        IntegerId subId,
        void* subContext,
        UA_StatusChangeNotification* notification
    ) noexcept {
        if (subContext != nullptr && notification != nullptr) {
            auto* self = static_cast<SubscriptionContext*>(subContext);
            self->invoke(
                self->statusChangeCallback,
                subId,
                asWrapper<StatusChangeNotification>(*notification)
            );
        }
    }

    static void deleteCallbackNative(
        [[maybe_unused]] UA_Client* client, IntegerId subId, void* subContext
    ) noexcept {
        if (subContext != nullptr) {
            auto* self = static_cast<SubscriptionContext*>(subContext);
            self->invoke(self->deleteCallback, subId);
            self->stale = true;
        }
    }
};

}  // namespace opcua::services::detail

#endif
