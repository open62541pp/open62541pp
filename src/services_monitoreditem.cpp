#include "open62541pp/services/monitoreditem.hpp"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include <algorithm>
#include <cstddef>

#include "open62541pp/client.hpp"
#include "open62541pp/detail/client_context.hpp"
#include "open62541pp/detail/exceptioncatcher.hpp"
#include "open62541pp/detail/server_context.hpp"
#include "open62541pp/server.hpp"

namespace opcua::services {

namespace detail {

template <typename T>
static auto createMonitoredItemContext(
    T& connection,
    const ReadValueId& itemToMonitor,
    DataChangeNotificationCallback dataChangeCallback,
    EventNotificationCallback eventCallback,
    DeleteMonitoredItemCallback deleteCallback
) {
    auto context = std::make_unique<MonitoredItemContext>();
    context->catcher = &opcua::detail::getExceptionCatcher(connection);
    context->itemToMonitor = itemToMonitor;
    context->dataChangeCallback = std::move(dataChangeCallback);
    context->eventCallback = std::move(eventCallback);
    context->deleteCallback = std::move(deleteCallback);
    return context;
}

std::vector<std::unique_ptr<MonitoredItemContext>> createMonitoredItemContexts(
    Client& connection,
    const CreateMonitoredItemsRequest& request,
    const DataChangeNotificationCallback& dataChangeCallback,
    const EventNotificationCallback& eventCallback,
    const DeleteMonitoredItemCallback& deleteCallback
) {
    const auto items = request.getItemsToCreate();
    std::vector<std::unique_ptr<MonitoredItemContext>> contexts(items.size());
    std::transform(items.begin(), items.end(), contexts.begin(), [&](const auto& item) {
        return createMonitoredItemContext(
            connection, item.getItemToMonitor(), dataChangeCallback, eventCallback, deleteCallback
        );
    });
    return contexts;
}

void convertMonitoredItemContexts(
    Span<const std::unique_ptr<MonitoredItemContext>> contexts,
    Span<MonitoredItemContext*> contextsPtr,
    Span<UA_Client_DataChangeNotificationCallback> dataChangeCallbacksNative,
    Span<UA_Client_EventNotificationCallback> eventCallbacksNative,
    Span<UA_Client_DeleteMonitoredItemCallback> deleteCallbacksNative
) noexcept {
    assert(contextsPtr.size() == contexts.size());
    std::transform(
        contexts.begin(),
        contexts.end(),
        contextsPtr.begin(),
        [](const auto& context) noexcept { return context.get(); }
    );
    if (!dataChangeCallbacksNative.empty()) {
        assert(dataChangeCallbacksNative.size() == contexts.size());
        std::fill(
            dataChangeCallbacksNative.begin(),
            dataChangeCallbacksNative.end(),
            MonitoredItemContext::dataChangeCallbackNativeClient
        );
    }
    if (!eventCallbacksNative.empty()) {
        assert(eventCallbacksNative.size() == contexts.size());
        std::fill(
            eventCallbacksNative.begin(),
            eventCallbacksNative.end(),
            MonitoredItemContext::eventCallbackNative
        );
    }
    if (!deleteCallbacksNative.empty()) {
        assert(deleteCallbacksNative.size() == contexts.size());
        std::fill(
            deleteCallbacksNative.begin(),
            deleteCallbacksNative.end(),
            MonitoredItemContext::deleteCallbackNative
        );
    }
}

template <typename T>
static void storeMonitoredItemContext(
    T& connection,
    uint32_t subscriptionId,
    const MonitoredItemCreateResult& result,
    std::unique_ptr<MonitoredItemContext>& context
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

void storeMonitoredItemContexts(
    Client& connection,
    uint32_t subscriptionId,
    const CreateMonitoredItemsResponse& response,
    Span<std::unique_ptr<MonitoredItemContext>> contexts
) {
    if (getServiceResult(response).isGood()) {
        const auto results = response.getResults();
        for (size_t i = 0; i < results.size(); ++i) {
            storeMonitoredItemContext(connection, subscriptionId, results[i], contexts[i]);
        }
    }
}

}  // namespace detail

CreateMonitoredItemsResponse createMonitoredItemsDataChange(
    Client& connection,
    const CreateMonitoredItemsRequest& request,
    DataChangeNotificationCallback dataChangeCallback,  // NOLINT
    DeleteMonitoredItemCallback deleteCallback  // NOLINT
) {
    // TODO: avoid heap allocations for single item?
    auto contexts = detail::createMonitoredItemContexts(
        connection, request, dataChangeCallback, {}, deleteCallback
    );
    std::vector<detail::MonitoredItemContext*> contextsPtr(contexts.size());
    std::vector<UA_Client_DataChangeNotificationCallback> dataChangeCallbacks(contexts.size());
    std::vector<UA_Client_DeleteMonitoredItemCallback> deleteCallbacks(contexts.size());
    detail::convertMonitoredItemContexts(
        contexts, contextsPtr, dataChangeCallbacks, {}, deleteCallbacks
    );
    CreateMonitoredItemsResponse response = UA_Client_MonitoredItems_createDataChanges(
        connection.handle(),
        request,
        reinterpret_cast<void**>(contextsPtr.data()),  // NOLINT
        dataChangeCallbacks.data(),
        deleteCallbacks.data()
    );
    detail::storeMonitoredItemContexts(connection, request.getSubscriptionId(), response, contexts);
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
    auto item = detail::createMonitoredItemCreateRequest(itemToMonitor, monitoringMode, parameters);
    const auto request = detail::createCreateMonitoredItemsRequest(
        subscriptionId, parameters.timestamps, {&item, 1}
    );
    auto response = createMonitoredItemsDataChange(
        connection,
        asWrapper<CreateMonitoredItemsRequest>(request),
        std::move(dataChangeCallback),
        std::move(deleteCallback)
    );
    return detail::wrapSingleResultWithStatus<MonitoredItemCreateResult>(response);
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
    auto context = detail::createMonitoredItemContext(
        connection, itemToMonitor, std::move(dataChangeCallback), {}, std::move(deleteCallback)
    );
    MonitoredItemCreateResult result = UA_Server_createDataChangeMonitoredItem(
        connection.handle(),
        static_cast<UA_TimestampsToReturn>(parameters.timestamps),
        detail::createMonitoredItemCreateRequest(itemToMonitor, monitoringMode, parameters),
        context.get(),
        detail::MonitoredItemContext::dataChangeCallbackNativeServer
    );
    detail::storeMonitoredItemContext(connection, 0U, result, context);
    return result;
}

CreateMonitoredItemsResponse createMonitoredItemsEvent(
    Client& connection,
    const CreateMonitoredItemsRequest& request,
    EventNotificationCallback eventCallback,  // NOLINT
    DeleteMonitoredItemCallback deleteCallback  // NOLINT
) {
    // TODO: avoid heap allocations for single item?
    auto contexts = detail::createMonitoredItemContexts(
        connection, request, {}, eventCallback, deleteCallback
    );
    std::vector<detail::MonitoredItemContext*> contextsPtr(contexts.size());
    std::vector<UA_Client_EventNotificationCallback> eventCallbacks(contexts.size());
    std::vector<UA_Client_DeleteMonitoredItemCallback> deleteCallbacks(contexts.size());
    detail::convertMonitoredItemContexts(
        contexts, contextsPtr, {}, eventCallbacks, deleteCallbacks
    );
    CreateMonitoredItemsResponse response = UA_Client_MonitoredItems_createEvents(
        connection.handle(),
        request,
        reinterpret_cast<void**>(contextsPtr.data()),  // NOLINT
        eventCallbacks.data(),
        deleteCallbacks.data()
    );
    detail::storeMonitoredItemContexts(connection, request.getSubscriptionId(), response, contexts);
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
    auto item = detail::createMonitoredItemCreateRequest(itemToMonitor, monitoringMode, parameters);
    const auto request = detail::createCreateMonitoredItemsRequest(
        subscriptionId, parameters.timestamps, {&item, 1}
    );
    auto response = createMonitoredItemsEvent(
        connection,
        asWrapper<CreateMonitoredItemsRequest>(request),
        std::move(eventCallback),
        std::move(deleteCallback)
    );
    return detail::wrapSingleResultWithStatus<MonitoredItemCreateResult>(response);
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
    const auto request = detail::createDeleteMonitoredItemsRequest(
        subscriptionId, {&monitoredItemId, 1}
    );
    return detail::getSingleStatus(
        deleteMonitoredItems(connection, asWrapper<DeleteMonitoredItemsRequest>(request))
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
