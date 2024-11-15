#pragma once

#include <cstdint>
#include <functional>

#include "open62541pp/services/detail/callbackadapter.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/types_composed.hpp"  // DataValue, IntegerId, Variant
#include "open62541pp/wrapper.hpp"  // asWrapper

struct UA_Client;
struct UA_Server;

namespace opcua::services::detail {

struct MonitoredItemContext : CallbackAdapter {
    bool stale{false};
    bool inserted{false};
    ReadValueId itemToMonitor;
    std::function<void(IntegerId subId, IntegerId monId, const DataValue&)> dataChangeCallback;
    std::function<void(IntegerId subId, IntegerId monId, Span<const Variant>)> eventCallback;
    std::function<void(IntegerId subId, IntegerId monId)> deleteCallback;

    static void dataChangeCallbackNativeServer(
        [[maybe_unused]] UA_Server* server,
        IntegerId monId,
        void* monContext,
        [[maybe_unused]] const UA_NodeId* nodeId,
        [[maybe_unused]] void* nodeContext,
        [[maybe_unused]] uint32_t attributeId,
        const UA_DataValue* value
    ) noexcept {
        if (monContext != nullptr && value != nullptr) {
            auto* self = static_cast<MonitoredItemContext*>(monContext);
            if (!self->inserted) {
                return;  // avoid immediate callbacks before insertion
            }
            self->invoke(self->dataChangeCallback, 0U, monId, asWrapper<DataValue>(*value));
        }
    }

    static void dataChangeCallbackNativeClient(
        [[maybe_unused]] UA_Client* client,
        IntegerId subId,
        [[maybe_unused]] void* subContext,
        IntegerId monId,
        void* monContext,
        UA_DataValue* value
    ) noexcept {
        if (monContext != nullptr && value != nullptr) {
            auto* self = static_cast<MonitoredItemContext*>(monContext);
            if (!self->inserted) {
                return;  // avoid immediate callbacks before insertion
            }
            self->invoke(self->dataChangeCallback, subId, monId, asWrapper<DataValue>(*value));
        }
    }

    static void eventCallbackNative(
        [[maybe_unused]] UA_Client* client,
        IntegerId subId,
        [[maybe_unused]] void* subContext,
        IntegerId monId,
        void* monContext,
        size_t nEventFields,
        UA_Variant* eventFields
    ) noexcept {
        if (monContext != nullptr) {
            auto* self = static_cast<MonitoredItemContext*>(monContext);
            if (!self->inserted) {
                return;  // avoid immediate callbacks before insertion
            }
            self->invoke(
                self->eventCallback,
                subId,
                monId,
                Span<const Variant>{asWrapper<Variant>(eventFields), nEventFields}
            );
        }
    }

    static void deleteCallbackNative(
        [[maybe_unused]] UA_Client* client,
        IntegerId subId,
        [[maybe_unused]] void* subContext,
        IntegerId monId,
        void* monContext
    ) noexcept {
        if (monContext != nullptr) {
            auto* self = static_cast<MonitoredItemContext*>(monContext);
            self->invoke(self->deleteCallback, subId, monId);
            self->stale = true;
        }
    }
};

}  // namespace opcua::services::detail
