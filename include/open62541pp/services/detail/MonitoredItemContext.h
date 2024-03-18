#pragma once

#include <cstdint>
#include <functional>

#include "open62541pp/Span.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/Staleable.h"
#include "open62541pp/services/detail/CallbackAdapter.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/Variant.h"

// forward declare
struct UA_Client;
struct UA_Server;

namespace opcua::services::detail {

struct MonitoredItemContext : CallbackAdapter, opcua::detail::Staleable {
    bool inserted{false};
    ReadValueId itemToMonitor;
    std::function<void(uint32_t subId, uint32_t monId, const DataValue&)> dataChangeCallback;
    std::function<void(uint32_t subId, uint32_t monId, Span<const Variant>)> eventCallback;
    std::function<void(uint32_t subId, uint32_t monId)> deleteCallback;

    static void dataChangeCallbackNativeServer(
        [[maybe_unused]] UA_Server* server,
        uint32_t monId,
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
        uint32_t subId,
        [[maybe_unused]] void* subContext,
        uint32_t monId,
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
        uint32_t subId,
        [[maybe_unused]] void* subContext,
        uint32_t monId,
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
        uint32_t subId,
        [[maybe_unused]] void* subContext,
        uint32_t monId,
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
