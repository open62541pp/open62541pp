#include "open62541pp/services/MonitoredItem.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include <algorithm>  // for_each_n
#include <cstddef>
#include <memory>
#include <utility>  // move

#include "open62541pp/Client.h"
#include "open62541pp/Common.h"  // MonitoringMode
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

uint32_t createMonitoredItemDataChange(
    Client& client,
    uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    MonitoringParameters& parameters,
    DataChangeNotificationCallback dataChangeCallback,
    DeleteMonitoredItemCallback deleteCallback
) {
    auto context = std::make_unique<detail::MonitoredItemContext>();
    context->catcher = &opcua::detail::getContext(client).exceptionCatcher;
    context->itemToMonitor = itemToMonitor;
    context->dataChangeCallback = std::move(dataChangeCallback);
    context->deleteCallback = std::move(deleteCallback);

    using Result = TypeWrapper<UA_MonitoredItemCreateResult, UA_TYPES_MONITOREDITEMCREATERESULT>;
    const Result result = UA_Client_MonitoredItems_createDataChange(
        client.handle(),
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
    opcua::detail::getContext(client).monitoredItems.insert(
        {subscriptionId, monitoredItemId}, std::move(context)
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
    auto context = std::make_unique<detail::MonitoredItemContext>();
    context->catcher = &opcua::detail::getContext(server).exceptionCatcher;
    context->itemToMonitor = itemToMonitor;
    context->dataChangeCallback = std::move(dataChangeCallback);

    using Result = TypeWrapper<UA_MonitoredItemCreateResult, UA_TYPES_MONITOREDITEMCREATERESULT>;
    const Result result = UA_Server_createDataChangeMonitoredItem(
        server.handle(),
        static_cast<UA_TimestampsToReturn>(parameters.timestamps),
        detail::createMonitoredItemCreateRequest(itemToMonitor, monitoringMode, parameters),
        context.get(),
        context->dataChangeCallbackNativeServer
    );
    throwIfBad(result->statusCode);
    detail::reviseMonitoringParameters(parameters, asNative(result));

    const auto monitoredItemId = result->monitoredItemId;
    opcua::detail::getContext(server).monitoredItems.insert(
        {0U, monitoredItemId}, std::move(context)
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
    auto context = std::make_unique<detail::MonitoredItemContext>();
    context->catcher = &opcua::detail::getContext(client).exceptionCatcher;
    context->itemToMonitor = itemToMonitor;
    context->eventCallback = std::move(eventCallback);
    context->deleteCallback = std::move(deleteCallback);

    using Result = TypeWrapper<UA_MonitoredItemCreateResult, UA_TYPES_MONITOREDITEMCREATERESULT>;
    const Result result = UA_Client_MonitoredItems_createEvent(
        client.handle(),
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
    opcua::detail::getContext(client).monitoredItems.insert(
        {subscriptionId, monitoredItemId}, std::move(context)
    );
    return monitoredItemId;
}

void modifyMonitoredItem(
    Client& client,
    uint32_t subscriptionId,
    uint32_t monitoredItemId,
    MonitoringParameters& parameters
) {
    auto item = detail::createMonitoredItemModifyRequest(monitoredItemId, parameters);
    auto request = detail::createModifyMonitoredItemsRequest(subscriptionId, parameters, item);
    using Response =
        TypeWrapper<UA_ModifyMonitoredItemsResponse, UA_TYPES_MODIFYMONITOREDITEMSRESPONSE>;
    const Response response = UA_Client_MonitoredItems_modify(client.handle(), request);
    auto& result = detail::getSingleResult(asNative(response));
    throwIfBad(result.statusCode);
    detail::reviseMonitoringParameters(parameters, result);
}

void setMonitoringMode(
    Client& client, uint32_t subscriptionId, uint32_t monitoredItemId, MonitoringMode monitoringMode
) {
    detail::sendRequest<UA_SetMonitoringModeRequest, UA_SetMonitoringModeResponse>(
        client,
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
    Client& client,
    uint32_t subscriptionId,
    uint32_t triggeringItemId,
    Span<const uint32_t> linksToAdd,
    Span<const uint32_t> linksToRemove
) {
    detail::sendRequest<UA_SetTriggeringRequest, UA_SetTriggeringResponse>(
        client,
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

void deleteMonitoredItem(Client& client, uint32_t subscriptionId, uint32_t monitoredItemId) {
    const auto status = UA_Client_MonitoredItems_deleteSingle(
        client.handle(), subscriptionId, monitoredItemId
    );
    throwIfBad(status);
}

void deleteMonitoredItem(Server& server, uint32_t monitoredItemId) {
    const auto status = UA_Server_deleteMonitoredItem(server.handle(), monitoredItemId);
    throwIfBad(status);
    opcua::detail::getContext(server).monitoredItems.erase({0U, monitoredItemId});
}

}  // namespace opcua::services

#endif
