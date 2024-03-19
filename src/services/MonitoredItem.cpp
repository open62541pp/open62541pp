#include "open62541pp/services/MonitoredItem.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include <algorithm>  // for_each_n
#include <cstddef>
#include <memory>
#include <utility>  // move

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/ClientContext.h"
#include "open62541pp/detail/ServerContext.h"
#include "open62541pp/detail/open62541/client.h"
#include "open62541pp/detail/open62541/server.h"
#include "open62541pp/services/detail/ClientService.h"
#include "open62541pp/services/detail/RequestHandling.h"
#include "open62541pp/services/detail/ResponseHandling.h"
#include "open62541pp/types/Composed.h"  // ReadValueId
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/Variant.h"

namespace opcua::services {

template <>
uint32_t createMonitoredItemDataChange<Client>(
    Client& connection,
    uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    MonitoringParametersEx& parameters,
    DataChangeNotificationCallback dataChangeCallback,
    DeleteMonitoredItemCallback deleteCallback
) {
    auto context = std::make_unique<detail::MonitoredItemContext>();
    context->catcher = &opcua::detail::getContext(connection).exceptionCatcher;
    context->itemToMonitor = itemToMonitor;
    context->dataChangeCallback = std::move(dataChangeCallback);
    context->deleteCallback = std::move(deleteCallback);
    auto* contextPtr = context.get();

    using Result = TypeWrapper<UA_MonitoredItemCreateResult, UA_TYPES_MONITOREDITEMCREATERESULT>;
    const Result result = UA_Client_MonitoredItems_createDataChange(
        connection.handle(),
        subscriptionId,
        static_cast<UA_TimestampsToReturn>(parameters.timestamps),
        detail::createMonitoredItemCreateRequest(itemToMonitor, monitoringMode, parameters),
        context.get(),
        context->dataChangeCallbackNativeClient,
        context->deleteCallbackNative
    );
    throwIfBad(result->statusCode);
    detail::reviseMonitoringParameters(parameters, asNative(result));

    const auto monitoredItemId = result->monitoredItemId;
    opcua::detail::getContext(connection)
        .monitoredItems.insert({subscriptionId, monitoredItemId}, std::move(context));
    contextPtr->inserted = true;
    return monitoredItemId;
}

template <>
uint32_t createMonitoredItemDataChange<Server>(
    Server& connection,
    [[maybe_unused]] uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    MonitoringParametersEx& parameters,
    DataChangeNotificationCallback dataChangeCallback,
    DeleteMonitoredItemCallback deleteCallback
) {
    auto context = std::make_unique<detail::MonitoredItemContext>();
    context->catcher = &opcua::detail::getContext(connection).exceptionCatcher;
    context->itemToMonitor = itemToMonitor;
    context->dataChangeCallback = std::move(dataChangeCallback);
    context->deleteCallback = std::move(deleteCallback);
    auto* contextPtr = context.get();

    using Result = TypeWrapper<UA_MonitoredItemCreateResult, UA_TYPES_MONITOREDITEMCREATERESULT>;
    const Result result = UA_Server_createDataChangeMonitoredItem(
        connection.handle(),
        static_cast<UA_TimestampsToReturn>(parameters.timestamps),
        detail::createMonitoredItemCreateRequest(itemToMonitor, monitoringMode, parameters),
        context.get(),
        context->dataChangeCallbackNativeServer
    );
    throwIfBad(result->statusCode);
    detail::reviseMonitoringParameters(parameters, asNative(result));

    const auto monitoredItemId = result->monitoredItemId;
    opcua::detail::getContext(connection)
        .monitoredItems.insert({0U, monitoredItemId}, std::move(context));
    contextPtr->inserted = true;
    return monitoredItemId;
}

[[nodiscard]] uint32_t createMonitoredItemDataChange(
    Server& connection,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    MonitoringParametersEx& parameters,
    DataChangeNotificationCallback dataChangeCallback
) {
    return createMonitoredItemDataChange(
        connection, 0U, itemToMonitor, monitoringMode, parameters, std::move(dataChangeCallback)
    );
}

uint32_t createMonitoredItemEvent(
    Client& connection,
    uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    MonitoringParametersEx& parameters,
    EventNotificationCallback eventCallback,
    DeleteMonitoredItemCallback deleteCallback
) {
    auto context = std::make_unique<detail::MonitoredItemContext>();
    context->catcher = &opcua::detail::getContext(connection).exceptionCatcher;
    context->itemToMonitor = itemToMonitor;
    context->eventCallback = std::move(eventCallback);
    context->deleteCallback = std::move(deleteCallback);
    auto* contextPtr = context.get();

    using Result = TypeWrapper<UA_MonitoredItemCreateResult, UA_TYPES_MONITOREDITEMCREATERESULT>;
    const Result result = UA_Client_MonitoredItems_createEvent(
        connection.handle(),
        subscriptionId,
        static_cast<UA_TimestampsToReturn>(parameters.timestamps),
        detail::createMonitoredItemCreateRequest(itemToMonitor, monitoringMode, parameters),
        context.get(),
        context->eventCallbackNative,
        context->deleteCallbackNative
    );
    throwIfBad(result->statusCode);
    detail::reviseMonitoringParameters(parameters, asNative(result));

    const auto monitoredItemId = result->monitoredItemId;
    opcua::detail::getContext(connection)
        .monitoredItems.insert({subscriptionId, monitoredItemId}, std::move(context));
    contextPtr->inserted = true;
    return monitoredItemId;
}

void modifyMonitoredItem(
    Client& connection,
    uint32_t subscriptionId,
    uint32_t monitoredItemId,
    MonitoringParametersEx& parameters
) {
    auto item = detail::createMonitoredItemModifyRequest(monitoredItemId, parameters);
    auto request = detail::createModifyMonitoredItemsRequest(subscriptionId, parameters, item);
    using Response =
        TypeWrapper<UA_ModifyMonitoredItemsResponse, UA_TYPES_MODIFYMONITOREDITEMSRESPONSE>;
    const Response response = UA_Client_MonitoredItems_modify(connection.handle(), request);
    auto& result = detail::getSingleResult(asNative(response));
    throwIfBad(result.statusCode);
    detail::reviseMonitoringParameters(parameters, result);
}

void setMonitoringMode(
    Client& connection,
    uint32_t subscriptionId,
    uint32_t monitoredItemId,
    MonitoringMode monitoringMode
) {
    detail::sendRequest<UA_SetMonitoringModeRequest, UA_SetMonitoringModeResponse>(
        connection,
        detail::createSetMonitoringModeRequest(
            subscriptionId, {&monitoredItemId, 1}, monitoringMode
        ),
        [](UA_SetMonitoringModeResponse& response) {
            throwIfBad(detail::getSingleResult(response));
        },
        detail::SyncOperation{}
    );
}

void setTriggering(
    Client& connection,
    uint32_t subscriptionId,
    uint32_t triggeringItemId,
    Span<const uint32_t> linksToAdd,
    Span<const uint32_t> linksToRemove
) {
    detail::sendRequest<UA_SetTriggeringRequest, UA_SetTriggeringResponse>(
        connection,
        detail::createSetTriggeringRequest(
            subscriptionId, triggeringItemId, linksToAdd, linksToRemove
        ),
        [](UA_SetTriggeringResponse& response) {
            throwIfBad(response.responseHeader.serviceResult);
            std::for_each_n(response.addResults, response.addResultsSize, throwIfBad);
            std::for_each_n(response.removeResults, response.removeResultsSize, throwIfBad);
        },
        detail::SyncOperation{}
    );
}

template <>
void deleteMonitoredItem<Client>(
    Client& connection, uint32_t subscriptionId, uint32_t monitoredItemId
) {
    const auto status = UA_Client_MonitoredItems_deleteSingle(
        connection.handle(), subscriptionId, monitoredItemId
    );
    throwIfBad(status);
}

template <>
void deleteMonitoredItem<Server>(
    Server& connection, [[maybe_unused]] uint32_t subscriptionId, uint32_t monitoredItemId
) {
    const auto status = UA_Server_deleteMonitoredItem(connection.handle(), monitoredItemId);
    throwIfBad(status);
    opcua::detail::getContext(connection).monitoredItems.erase({0U, monitoredItemId});
}

void deleteMonitoredItem(Server& connection, uint32_t monitoredItemId) {
    deleteMonitoredItem(connection, 0U, monitoredItemId);
}

}  // namespace opcua::services

#endif
