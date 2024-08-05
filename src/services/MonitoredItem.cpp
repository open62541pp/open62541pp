#include "open62541pp/services/MonitoredItem.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include <algorithm>  // for_each_n
#include <cstddef>
#include <memory>
#include <utility>  // move

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/Span.h"
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

template <typename T>
inline static auto createMonitoredItemContext(
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
inline static Result<uint32_t> createMonitoredItem(
    T& connection,
    uint32_t subscriptionId,
    MonitoringParametersEx& parameters,
    std::unique_ptr<detail::MonitoredItemContext> context,
    const UA_MonitoredItemCreateResult& result
) {
    if (StatusCode code = result.statusCode; code.isBad()) {
        return BadResult(code);
    }
    detail::reviseMonitoringParameters(parameters, result);

    const auto monitoredItemId = result.monitoredItemId;
    auto* contextPtr = context.get();
    opcua::detail::getContext(connection)
        .monitoredItems.insert({subscriptionId, monitoredItemId}, std::move(context));
    contextPtr->inserted = true;
    return monitoredItemId;
}

template <>
Result<uint32_t> createMonitoredItemDataChange<Client>(
    Client& connection,
    uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    MonitoringParametersEx& parameters,
    DataChangeNotificationCallback dataChangeCallback,
    DeleteMonitoredItemCallback deleteCallback
) {
    auto context = createMonitoredItemContext(
        connection, itemToMonitor, std::move(dataChangeCallback), {}, std::move(deleteCallback)
    );
    const MonitoredItemCreateResult result = UA_Client_MonitoredItems_createDataChange(
        connection.handle(),
        subscriptionId,
        static_cast<UA_TimestampsToReturn>(parameters.timestamps),
        detail::createMonitoredItemCreateRequest(itemToMonitor, monitoringMode, parameters),
        context.get(),
        context->dataChangeCallbackNativeClient,
        context->deleteCallbackNative
    );
    return createMonitoredItem(connection, subscriptionId, parameters, std::move(context), result);
}

template <>
Result<uint32_t> createMonitoredItemDataChange<Server>(
    Server& connection,
    [[maybe_unused]] uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    MonitoringParametersEx& parameters,
    DataChangeNotificationCallback dataChangeCallback,
    DeleteMonitoredItemCallback deleteCallback
) {
    auto context = createMonitoredItemContext(
        connection, itemToMonitor, std::move(dataChangeCallback), {}, std::move(deleteCallback)
    );
    const MonitoredItemCreateResult result = UA_Server_createDataChangeMonitoredItem(
        connection.handle(),
        static_cast<UA_TimestampsToReturn>(parameters.timestamps),
        detail::createMonitoredItemCreateRequest(itemToMonitor, monitoringMode, parameters),
        context.get(),
        context->dataChangeCallbackNativeServer
    );
    return createMonitoredItem(connection, 0U, parameters, std::move(context), result);
}

Result<uint32_t> createMonitoredItemEvent(
    Client& connection,
    uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    MonitoringParametersEx& parameters,
    EventNotificationCallback eventCallback,
    DeleteMonitoredItemCallback deleteCallback
) {
    auto context = createMonitoredItemContext(
        connection, itemToMonitor, {}, std::move(eventCallback), std::move(deleteCallback)
    );
    const MonitoredItemCreateResult result = UA_Client_MonitoredItems_createEvent(
        connection.handle(),
        subscriptionId,
        static_cast<UA_TimestampsToReturn>(parameters.timestamps),
        detail::createMonitoredItemCreateRequest(itemToMonitor, monitoringMode, parameters),
        context.get(),
        context->eventCallbackNative,
        context->deleteCallbackNative
    );
    return createMonitoredItem(connection, subscriptionId, parameters, std::move(context), result);
}

Result<void> modifyMonitoredItem(
    Client& connection,
    uint32_t subscriptionId,
    uint32_t monitoredItemId,
    MonitoringParametersEx& parameters
) noexcept {
    auto item = detail::createMonitoredItemModifyRequest(monitoredItemId, parameters);
    auto request = detail::createModifyMonitoredItemsRequest(subscriptionId, parameters, item);
    const ModifyMonitoredItemsResponse response = UA_Client_MonitoredItems_modify(
        connection.handle(), request
    );
    return detail::getSingleResult(asNative(response))
        .andThen([&](UA_MonitoredItemModifyResult& result) -> Result<void> {
            if (StatusCode code = result.statusCode; code.isBad()) {
                return BadResult(code);
            }
            detail::reviseMonitoringParameters(parameters, result);
            return {};
        });
}

Result<void> setMonitoringMode(
    Client& connection,
    uint32_t subscriptionId,
    uint32_t monitoredItemId,
    MonitoringMode monitoringMode
) noexcept {
    return detail::sendRequest<UA_SetMonitoringModeRequest, UA_SetMonitoringModeResponse>(
        connection,
        detail::createSetMonitoringModeRequest(
            subscriptionId, {&monitoredItemId, 1}, monitoringMode
        ),
        [](UA_SetMonitoringModeResponse& response) -> Result<void> {
            return detail::getSingleResult(response).andThen(detail::toResult);
        },
        detail::SyncOperation{}
    );
}

Result<void> setTriggering(
    Client& connection,
    uint32_t subscriptionId,
    uint32_t triggeringItemId,
    Span<const uint32_t> linksToAdd,
    Span<const uint32_t> linksToRemove
) noexcept {
    return detail::sendRequest<UA_SetTriggeringRequest, UA_SetTriggeringResponse>(
        connection,
        detail::createSetTriggeringRequest(
            subscriptionId, triggeringItemId, linksToAdd, linksToRemove
        ),
        [](UA_SetTriggeringResponse& response) -> Result<void> {
            if (StatusCode code = response.responseHeader.serviceResult; code.isBad()) {
                return BadResult(code);
            }
            for (StatusCode code : Span(response.addResults, response.addResultsSize)) {
                if (code.isBad()) {
                    return BadResult(code);
                }
            }
            for (StatusCode code : Span(response.removeResults, response.removeResultsSize)) {
                if (code.isBad()) {
                    return BadResult(code);
                }
            }
            return {};
        },
        detail::SyncOperation{}
    );
}

template <>
Result<void> deleteMonitoredItem<Client>(
    Client& connection, uint32_t subscriptionId, uint32_t monitoredItemId
) {
    return detail::toResult(
        UA_Client_MonitoredItems_deleteSingle(connection.handle(), subscriptionId, monitoredItemId)
    );
}

template <>
Result<void> deleteMonitoredItem<Server>(
    Server& connection, [[maybe_unused]] uint32_t subscriptionId, uint32_t monitoredItemId
) {
    const auto result = detail::toResult(
        UA_Server_deleteMonitoredItem(connection.handle(), monitoredItemId)
    );
    opcua::detail::getContext(connection).monitoredItems.erase({0U, monitoredItemId});
    return result;
}

}  // namespace opcua::services

#endif
