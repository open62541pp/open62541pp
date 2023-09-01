#include "open62541pp/services/MonitoredItem.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include <cstddef>
#include <memory>
#include <type_traits>  // is_same_v
#include <utility>  // move

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/Variant.h"

#include "../ClientContext.h"
#include "../ServerContext.h"
#include "../open62541_impl.h"

namespace opcua::services {

static void dataChangeNotificationCallback(
    [[maybe_unused]] UA_Server* server,
    uint32_t monitoredItemId,
    void* monitoredItemContext,
    [[maybe_unused]] const UA_NodeId* nodeId,
    [[maybe_unused]] void* nodeContext,
    [[maybe_unused]] uint32_t attributeId,
    const UA_DataValue* value
) noexcept {
    if (monitoredItemContext == nullptr) {
        return;
    }
    auto* monitoredItem = static_cast<ServerContext::MonitoredItem*>(monitoredItemContext);
    auto& callback = monitoredItem->dataChangeCallback;
    if (callback) {
        detail::invokeCatchIgnore([&] {
            callback(0U, monitoredItemId, asWrapper<DataValue>(*value));
        });
    }
}

static void dataChangeNotificationCallback(
    [[maybe_unused]] UA_Client* client,
    uint32_t subId,
    [[maybe_unused]] void* subContext,
    uint32_t monId,
    void* monContext,
    UA_DataValue* value
) noexcept {
    if (monContext == nullptr) {
        return;
    }
    auto* monitoredItem = static_cast<ClientContext::MonitoredItem*>(monContext);
    auto& callback = monitoredItem->dataChangeCallback;
    if (callback) {
        detail::invokeCatchIgnore([&] { callback(subId, monId, asWrapper<DataValue>(*value)); });
    }
}

static void eventNotificationCallback(
    [[maybe_unused]] UA_Client* client,
    uint32_t subId,
    [[maybe_unused]] void* subContext,
    uint32_t monId,
    void* monContext,
    size_t nEventFields,
    UA_Variant* eventFields
) noexcept {
    if (monContext == nullptr) {
        return;
    }
    auto* monitoredItem = static_cast<ClientContext::MonitoredItem*>(monContext);
    auto& callback = monitoredItem->eventCallback;
    if (callback) {
        detail::invokeCatchIgnore([&] {
            callback(subId, monId, {asWrapper<Variant>(eventFields), nEventFields});
        });
    }
}

static void deleteMonitoredItemCallback(
    [[maybe_unused]] UA_Client* client,
    uint32_t subId,
    [[maybe_unused]] void* subContext,
    uint32_t monId,
    void* monContext
) noexcept {
    if (monContext != nullptr) {
        auto* monitoredItem = static_cast<ClientContext::MonitoredItem*>(monContext);
        if (monitoredItem->deleteCallback) {
            monitoredItem->deleteCallback(subId, monId);
        }
    }
    ClientContext& clientContext = getContext(client);
    clientContext.monitoredItems.erase({subId, monId});
}

inline static void copyMonitoringParametersToNative(
    const MonitoringParameters& parameters, UA_MonitoringParameters& native
) {
    native.samplingInterval = parameters.samplingInterval;
    native.filter = parameters.filter;
    native.queueSize = parameters.queueSize;
    native.discardOldest = parameters.discardOldest;
}

template <typename T>
inline static void reviseMonitoringParameters(MonitoringParameters& parameters, const T& result) {
    // response type may be UA_MonitoredItemCreateResult or UA_MonitoredItemModifyResult
    parameters.samplingInterval = result->revisedSamplingInterval;
    parameters.queueSize = result->revisedQueueSize;
    parameters.filter = asWrapper<ExtensionObject>(result->filterResult);
}

uint32_t createMonitoredItemDataChange(
    Client& client,
    uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    MonitoringParameters& parameters,
    DataChangeNotificationCallback dataChangeCallback,
    DeleteMonitoredItemCallback deleteCallback
) {
    UA_MonitoredItemCreateRequest request{};
    request.itemToMonitor = *itemToMonitor.handle();
    request.monitoringMode = static_cast<UA_MonitoringMode>(monitoringMode);
    copyMonitoringParametersToNative(parameters, request.requestedParameters);

    auto monitoredItemContext = std::make_unique<ClientContext::MonitoredItem>();
    monitoredItemContext->itemToMonitor = itemToMonitor;
    monitoredItemContext->dataChangeCallback = std::move(dataChangeCallback);
    monitoredItemContext->deleteCallback = std::move(deleteCallback);

    using Result = TypeWrapper<UA_MonitoredItemCreateResult, UA_TYPES_MONITOREDITEMCREATERESULT>;
    const Result result = UA_Client_MonitoredItems_createDataChange(
        client.handle(),
        subscriptionId,
        static_cast<UA_TimestampsToReturn>(parameters.timestamps),
        request,
        monitoredItemContext.get(),
        dataChangeNotificationCallback,
        deleteMonitoredItemCallback
    );
    detail::throwOnBadStatus(result->statusCode);
    reviseMonitoringParameters(parameters, result);

    const auto monitoredItemId = result->monitoredItemId;
    client.getContext().monitoredItems.insert_or_assign(
        {subscriptionId, monitoredItemId}, std::move(monitoredItemContext)
    );
    return monitoredItemId;
}

uint32_t createMonitoredItemDataChange(
    Server& server,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    MonitoringParameters& parameters,
    DataChangeNotificationCallback dataChangeCallback
) {
    UA_MonitoredItemCreateRequest request{};
    request.itemToMonitor = *itemToMonitor.handle();
    request.monitoringMode = static_cast<UA_MonitoringMode>(monitoringMode);
    copyMonitoringParametersToNative(parameters, request.requestedParameters);

    auto monitoredItemContext = std::make_unique<ServerContext::MonitoredItem>();
    monitoredItemContext->itemToMonitor = itemToMonitor;
    monitoredItemContext->dataChangeCallback = std::move(dataChangeCallback);

    using Result = TypeWrapper<UA_MonitoredItemCreateResult, UA_TYPES_MONITOREDITEMCREATERESULT>;
    const Result result = UA_Server_createDataChangeMonitoredItem(
        server.handle(),
        static_cast<UA_TimestampsToReturn>(parameters.timestamps),
        request,
        monitoredItemContext.get(),
        dataChangeNotificationCallback
    );
    detail::throwOnBadStatus(result->statusCode);
    reviseMonitoringParameters(parameters, result);

    const auto monitoredItemId = result->monitoredItemId;
    server.getContext().monitoredItems.insert_or_assign(
        monitoredItemId, std::move(monitoredItemContext)
    );
    return monitoredItemId;
}

uint32_t createMonitoredItemEvent(
    Client& client,
    uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    MonitoringParameters& parameters,
    EventNotificationCallback eventCallback,
    DeleteMonitoredItemCallback deleteCallback
) {
    UA_MonitoredItemCreateRequest request{};
    request.itemToMonitor = *itemToMonitor.handle();
    request.monitoringMode = static_cast<UA_MonitoringMode>(monitoringMode);
    copyMonitoringParametersToNative(parameters, request.requestedParameters);

    auto monitoredItemContext = std::make_unique<ClientContext::MonitoredItem>();
    monitoredItemContext->itemToMonitor = itemToMonitor;
    monitoredItemContext->eventCallback = std::move(eventCallback);
    monitoredItemContext->deleteCallback = std::move(deleteCallback);

    using Result = TypeWrapper<UA_MonitoredItemCreateResult, UA_TYPES_MONITOREDITEMCREATERESULT>;
    const Result result = UA_Client_MonitoredItems_createEvent(
        client.handle(),
        subscriptionId,
        static_cast<UA_TimestampsToReturn>(parameters.timestamps),
        request,
        monitoredItemContext.get(),
        eventNotificationCallback,
        deleteMonitoredItemCallback
    );
    detail::throwOnBadStatus(result->statusCode);
    reviseMonitoringParameters(parameters, result);

    const auto monitoredItemId = result->monitoredItemId;
    client.getContext().monitoredItems.insert_or_assign(
        {subscriptionId, monitoredItemId}, std::move(monitoredItemContext)
    );
    return monitoredItemId;
}

void modifyMonitoredItem(
    Client& client,
    uint32_t subscriptionId,
    uint32_t monitoredItemId,
    MonitoringParameters& parameters
) {
    UA_MonitoredItemModifyRequest itemToModify{};
    itemToModify.monitoredItemId = monitoredItemId;
    copyMonitoringParametersToNative(parameters, itemToModify.requestedParameters);

    UA_ModifyMonitoredItemsRequest request{};
    request.subscriptionId = subscriptionId;
    request.timestampsToReturn = static_cast<UA_TimestampsToReturn>(parameters.timestamps);
    request.itemsToModifySize = 1;
    request.itemsToModify = &itemToModify;

    using Response =
        TypeWrapper<UA_ModifyMonitoredItemsResponse, UA_TYPES_MODIFYMONITOREDITEMSRESPONSE>;
    const Response response = UA_Client_MonitoredItems_modify(client.handle(), request);
    detail::throwOnBadStatus(response->responseHeader.serviceResult);
    if (response->resultsSize != 1) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
    auto* result = response->results;
    detail::throwOnBadStatus(result->statusCode);
    reviseMonitoringParameters(parameters, result);
}

void setMonitoringMode(
    Client& client, uint32_t subscriptionId, uint32_t monitoredItemId, MonitoringMode monitoringMode
) {
    UA_SetMonitoringModeRequest request{};
    request.subscriptionId = subscriptionId;
    request.monitoringMode = static_cast<UA_MonitoringMode>(monitoringMode);
    request.monitoredItemIdsSize = 1;
    request.monitoredItemIds = &monitoredItemId;

    using Response = TypeWrapper<UA_SetMonitoringModeResponse, UA_TYPES_SETMONITORINGMODERESPONSE>;
    const Response response = UA_Client_MonitoredItems_setMonitoringMode(client.handle(), request);
    detail::throwOnBadStatus(response->responseHeader.serviceResult);
    if (response->resultsSize != 1) {
        throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
    }
    detail::throwOnBadStatus(*response->results);
}

void setTriggering(
    Client& client,
    uint32_t subscriptionId,
    uint32_t triggeringItemId,
    Span<const uint32_t> linksToAdd,
    Span<const uint32_t> linksToRemove
) {
    UA_SetTriggeringRequest request{};
    request.subscriptionId = subscriptionId;
    request.triggeringItemId = triggeringItemId;
    request.linksToAddSize = linksToAdd.size();
    request.linksToAdd = const_cast<uint32_t*>(linksToAdd.data());  // NOLINT
    request.linksToRemoveSize = linksToRemove.size();
    request.linksToRemove = const_cast<uint32_t*>(linksToRemove.data());  // NOLINT

    using Response = TypeWrapper<UA_SetTriggeringResponse, UA_TYPES_SETTRIGGERINGRESPONSE>;
    const Response response = UA_Client_MonitoredItems_setTriggering(client.handle(), request);
    detail::throwOnBadStatus(response->responseHeader.serviceResult);
    for (auto&& status : Span(response->addResults, response->addResultsSize)) {
        detail::throwOnBadStatus(status);
    }
    for (auto&& status : Span(response->removeResults, response->removeResultsSize)) {
        detail::throwOnBadStatus(status);
    }
}

void deleteMonitoredItem(Client& client, uint32_t subscriptionId, uint32_t monitoredItemId) {
    const auto status = UA_Client_MonitoredItems_deleteSingle(
        client.handle(), subscriptionId, monitoredItemId
    );
    detail::throwOnBadStatus(status);
}

void deleteMonitoredItem(Server& server, uint32_t monitoredItemId) {
    const auto status = UA_Server_deleteMonitoredItem(server.handle(), monitoredItemId);
    detail::throwOnBadStatus(status);
    server.getContext().monitoredItems.erase(monitoredItemId);
}

}  // namespace opcua::services

#endif
