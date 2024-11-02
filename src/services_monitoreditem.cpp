#include "open62541pp/services/monitoreditem.hpp"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include <cstddef>
#include <memory>
#include <utility>  // move
#include <vector>

#include "open62541pp/client.hpp"
#include "open62541pp/detail/client_context.hpp"
#include "open62541pp/detail/server_context.hpp"
#include "open62541pp/server.hpp"
#include "open62541pp/types.hpp"  // StatusCode

namespace opcua::services {

template <typename T>
static auto createMonitoredItemContext(
    T& connection,
    const ReadValueId& itemToMonitor,
    DataChangeNotificationCallback dataChangeCallback,
    EventNotificationCallback eventCallback,
    DeleteMonitoredItemCallback deleteCallback
) {
    auto context = std::make_unique<detail::MonitoredItemContext>();
    context->catcher = &opcua::detail::getContext(connection).exceptionCatcher;
    context->itemToMonitor = itemToMonitor;
    context->dataChangeCallback = std::move(dataChangeCallback);
    context->eventCallback = std::move(eventCallback);
    context->deleteCallback = std::move(deleteCallback);
    return context;
}

template <typename T>
static auto createMonitoredItemContexts(
    T& connection,
    const CreateMonitoredItemsRequest& request,
    const DataChangeNotificationCallback& dataChangeCallback,
    const EventNotificationCallback& eventCallback,
    const DeleteMonitoredItemCallback& deleteCallback
) {
    std::vector<std::unique_ptr<detail::MonitoredItemContext>> contexts;
    for (const auto& item : request.getItemsToCreate()) {
        contexts.push_back(createMonitoredItemContext(
            connection, item.getItemToMonitor(), dataChangeCallback, eventCallback, deleteCallback
        ));
    }
    return contexts;
}

template <typename T>
static void storeMonitoredItemContext(
    T& connection,
    uint32_t subscriptionId,
    const MonitoredItemCreateResult& result,
    std::unique_ptr<detail::MonitoredItemContext>& context
) {
    if (result.getStatusCode().isGood()) {
        auto* contextPtr = context.get();
        opcua::detail::getContext(connection)
            .monitoredItems.insert(
                {subscriptionId, result.getMonitoredItemId()}, std::move(context)
            );
        contextPtr->inserted = true;
    }
}

template <typename T>
static void storeMonitoredItemContexts(
    T& connection,
    uint32_t subscriptionId,
    const CreateMonitoredItemsResponse& response,
    Span<std::unique_ptr<detail::MonitoredItemContext>> contexts
) {
    if (detail::getServiceResult(response).isGood()) {
        const auto& results = response.getResults();
        for (size_t i = 0; i < results.size(); ++i) {
            storeMonitoredItemContext(connection, subscriptionId, results[i], contexts[i]);
        }
    }
}

CreateMonitoredItemsResponse createMonitoredItemsDataChange(
    Client& connection,
    const CreateMonitoredItemsRequest& request,
    DataChangeNotificationCallback dataChangeCallback,  // NOLINT
    DeleteMonitoredItemCallback deleteCallback  // NOLINT
) {
    auto contexts = createMonitoredItemContexts(
        connection, request, dataChangeCallback, {}, deleteCallback
    );
    std::vector<void*> contextsRaw;
    std::vector<UA_Client_DataChangeNotificationCallback> dataChangeCallbacks;
    std::vector<UA_Client_DeleteMonitoredItemCallback> deleteCallbacks;
    for (const auto& context : contexts) {
        contextsRaw.push_back(context.get());
        dataChangeCallbacks.push_back(context->dataChangeCallbackNativeClient);
        deleteCallbacks.push_back(context->deleteCallbackNative);
    }
    CreateMonitoredItemsResponse response = UA_Client_MonitoredItems_createDataChanges(
        connection.handle(),
        request,
        contextsRaw.data(),
        dataChangeCallbacks.data(),
        deleteCallbacks.data()
    );
    storeMonitoredItemContexts(connection, request.getSubscriptionId(), response, contexts);
    return response;
}

template <>
MonitoredItemCreateResult createMonitoredItemDataChange<Client>(
    Client& connection,
    uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    const MonitoringParametersEx& parameters,
    DataChangeNotificationCallback dataChangeCallback,
    DeleteMonitoredItemCallback deleteCallback
) {
    auto context = createMonitoredItemContext(
        connection, itemToMonitor, std::move(dataChangeCallback), {}, std::move(deleteCallback)
    );
    MonitoredItemCreateResult result = UA_Client_MonitoredItems_createDataChange(
        connection.handle(),
        subscriptionId,
        static_cast<UA_TimestampsToReturn>(parameters.timestamps),
        detail::createMonitoredItemCreateRequest(itemToMonitor, monitoringMode, parameters),
        context.get(),
        context->dataChangeCallbackNativeClient,
        context->deleteCallbackNative
    );
    storeMonitoredItemContext(connection, subscriptionId, result, context);
    return result;
}

template <>
MonitoredItemCreateResult createMonitoredItemDataChange<Server>(
    Server& connection,
    [[maybe_unused]] uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    const MonitoringParametersEx& parameters,
    DataChangeNotificationCallback dataChangeCallback,
    DeleteMonitoredItemCallback deleteCallback
) {
    auto context = createMonitoredItemContext(
        connection, itemToMonitor, std::move(dataChangeCallback), {}, std::move(deleteCallback)
    );
    MonitoredItemCreateResult result = UA_Server_createDataChangeMonitoredItem(
        connection.handle(),
        static_cast<UA_TimestampsToReturn>(parameters.timestamps),
        detail::createMonitoredItemCreateRequest(itemToMonitor, monitoringMode, parameters),
        context.get(),
        context->dataChangeCallbackNativeServer
    );
    storeMonitoredItemContext(connection, 0U, result, context);
    return result;
}

CreateMonitoredItemsResponse createMonitoredItemsEvent(
    Client& connection,
    const CreateMonitoredItemsRequest& request,
    EventNotificationCallback eventCallback,  // NOLINT
    DeleteMonitoredItemCallback deleteCallback  // NOLINT
) {
    auto contexts = createMonitoredItemContexts(
        connection, request, {}, eventCallback, deleteCallback
    );
    std::vector<void*> contextsRaw;
    std::vector<UA_Client_EventNotificationCallback> eventCallbacks;
    std::vector<UA_Client_DeleteMonitoredItemCallback> deleteCallbacks;
    for (const auto& context : contexts) {
        contextsRaw.push_back(context.get());
        eventCallbacks.push_back(context->eventCallbackNative);
        deleteCallbacks.push_back(context->deleteCallbackNative);
    }
    CreateMonitoredItemsResponse response = UA_Client_MonitoredItems_createEvents(
        connection.handle(),
        request,
        contextsRaw.data(),
        eventCallbacks.data(),
        deleteCallbacks.data()
    );
    storeMonitoredItemContexts(connection, request.getSubscriptionId(), response, contexts);
    return response;
}

MonitoredItemCreateResult createMonitoredItemEvent(
    Client& connection,
    uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    const MonitoringParametersEx& parameters,
    EventNotificationCallback eventCallback,
    DeleteMonitoredItemCallback deleteCallback
) {
    auto context = createMonitoredItemContext(
        connection, itemToMonitor, {}, std::move(eventCallback), std::move(deleteCallback)
    );
    MonitoredItemCreateResult result = UA_Client_MonitoredItems_createEvent(
        connection.handle(),
        subscriptionId,
        static_cast<UA_TimestampsToReturn>(parameters.timestamps),
        detail::createMonitoredItemCreateRequest(itemToMonitor, monitoringMode, parameters),
        context.get(),
        context->eventCallbackNative,
        context->deleteCallbackNative
    );
    storeMonitoredItemContext(connection, subscriptionId, result, context);
    return result;
}

ModifyMonitoredItemsResponse modifyMonitoredItems(
    Client& connection, const ModifyMonitoredItemsRequest& request
) noexcept {
    return UA_Client_MonitoredItems_modify(connection.handle(), request);
}

SetMonitoringModeResponse setMonitoringMode(
    Client& connection, const SetMonitoringModeRequest& request
) noexcept {
    return UA_Client_MonitoredItems_setMonitoringMode(connection.handle(), request);
}

SetTriggeringResponse setTriggering(
    Client& connection, const SetTriggeringRequest& request
) noexcept {
    return UA_Client_MonitoredItems_setTriggering(connection.handle(), request);
}

DeleteMonitoredItemsResponse deleteMonitoredItems(
    Client& connection, const DeleteMonitoredItemsRequest& request
) noexcept {
    return UA_Client_MonitoredItems_delete(connection.handle(), request);
}

template <>
StatusCode deleteMonitoredItem<Client>(
    Client& connection, uint32_t subscriptionId, uint32_t monitoredItemId
) {
    return UA_Client_MonitoredItems_deleteSingle(
        connection.handle(), subscriptionId, monitoredItemId
    );
}

template <>
StatusCode deleteMonitoredItem<Server>(
    Server& connection, [[maybe_unused]] uint32_t subscriptionId, uint32_t monitoredItemId
) {
    const auto status = UA_Server_deleteMonitoredItem(connection.handle(), monitoredItemId);
    opcua::detail::getContext(connection).monitoredItems.erase({0U, monitoredItemId});
    return status;
}

}  // namespace opcua::services

#endif
