#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <utility>  // forward
#include <vector>

#include "open62541pp/async.hpp"
#include "open62541pp/common.hpp"  // TimestampsToReturn, MonitoringMode
#include "open62541pp/config.hpp"
#include "open62541pp/detail/client_utils.hpp"  // getHandle
#include "open62541pp/services/detail/async_hook.hpp"
#include "open62541pp/services/detail/async_transform.hpp"
#include "open62541pp/services/detail/client_service.hpp"
#include "open62541pp/services/detail/monitoreditem_context.hpp"
#include "open62541pp/services/detail/request_handling.hpp"
#include "open62541pp/services/detail/response_handling.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/types_composed.hpp"

#ifdef UA_ENABLE_SUBSCRIPTIONS

namespace opcua {
class Client;
}  // namespace opcua

namespace opcua::services {

/**
 * @defgroup MonitoredItem MonitoredItem service set
 * Subscribe to data and events.
 *
 * Note the difference between Subscriptions and MonitoredItems. Subscriptions are used to report
 * back notifications. Monitored items are used to generate notifications. Every monitored item is
 * attached to exactly one subscription. And a subscription can contain many monitored items.
 *
 * Monitored items can also be registered locally (server-side). Notifications are then forwarded
 * to a user-defined callback instead of a remote client.
 *
 * @see Subscription
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12
 * @ingroup Services
 * @{
 */

/**
 * Extended monitoring parameters with default values from open62541.
 * This is an extended version of `UA_MonitoringParameters` with the `timestamps` parameter.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.21
 */
struct MonitoringParametersEx {
    /// Timestamps to be transmitted.
    TimestampsToReturn timestamps = TimestampsToReturn::Both;
    /// Interval in milliseconds that defines the fastest rate at which the MonitoredItem should be
    /// accessed and evaluated. The following values have special meaning:
    /// - `0.0` to use the fastest practical rate
    /// - `-1.0` to use the default sampling interval (publishing interval of the subscription)
    double samplingInterval = 250.0;
    /// Filter is used by the server to determine if the MonitoredItem should generate
    /// notifications. The filter parameter type is an extensible parameter type and can be, for
    /// example, of type DataChangeFilter, EventFilter or AggregateFilter.
    /// @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.22
    ExtensionObject filter;
    /// Size of the MonitoringItem queue.
    /// The following values have special meaning:
    /// - `0` to retrieve the server's default queue size
    /// - `1` to retrieve the server's minimum queue size
    /// In the case of a queue overflow, an Event of the type EventQueueOverflowEventType is
    /// generated.
    uint32_t queueSize = 1;
    /// Discard policy when the queue is full.
    /// - `true`: the oldest (first) notification in the queue is discarded
    /// - `false`: the last notification added to the queue gets replaced with the new notification
    bool discardOldest = true;
};

/**
 * @defgroup CreateMonitoredItems CreateMonitoredItems service
 * Create and add a monitored item to a subscription.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.2
 * @{
 */

/**
 * MonitoredItem deletion callback.
 * @param subId Subscription identifier
 * @param monId MonitoredItem identifier
 */
using DeleteMonitoredItemCallback = std::function<void(uint32_t subId, uint32_t monId)>;

/**
 * Data change notification callback.
 * @param subId Subscription identifier (`0U` for local (server-side) monitored item)
 * @param monId MonitoredItem identifier
 * @param value Changed value
 */
using DataChangeNotificationCallback =
    std::function<void(uint32_t subId, uint32_t monId, const DataValue& value)>;

/**
 * Event notification callback.
 * @param subId Subscription identifier (`0U` for local (server-side) monitored item)
 * @param monId MonitoredItem identifier
 * @param eventFields Event fields
 */
using EventNotificationCallback =
    std::function<void(uint32_t subId, uint32_t monId, Span<const Variant> eventFields)>;

namespace detail {
std::vector<std::unique_ptr<MonitoredItemContext>> createMonitoredItemContexts(
    Client& connection,
    const CreateMonitoredItemsRequest& request,
    DataChangeNotificationCallback dataChangeCallback,
    EventNotificationCallback eventCallback,
    DeleteMonitoredItemCallback deleteCallback
);

void convertMonitoredItemContexts(
    Span<const std::unique_ptr<MonitoredItemContext>> contexts,
    Span<MonitoredItemContext*> contextsPtr,
    Span<UA_Client_DataChangeNotificationCallback> dataChangeCallbacksNative,
    Span<UA_Client_EventNotificationCallback> eventCallbacksNative,
    Span<UA_Client_DeleteMonitoredItemCallback> deleteCallbacksNative
) noexcept;

void storeMonitoredItemContexts(
    Client& connection,
    uint32_t subscriptionId,
    const CreateMonitoredItemsResponse& response,
    Span<std::unique_ptr<MonitoredItemContext>> contexts
);
}  // namespace detail

/**
 * Create and add monitored items to a subscription for data change notifications.
 * Don't use this function to monitor the `EventNotifier` attribute.
 * Create monitored items with @ref createMonitoredItemsEvent instead.
 *
 * @param connection Instance of type Client
 * @param request Create monitored items request
 * @param dataChangeCallback Invoked when the monitored item is changed
 * @param deleteCallback Invoked when the monitored item is deleted
 */
[[nodiscard]] CreateMonitoredItemsResponse createMonitoredItemsDataChange(
    Client& connection,
    const CreateMonitoredItemsRequest& request,
    DataChangeNotificationCallback dataChangeCallback,
    DeleteMonitoredItemCallback deleteCallback
);

#if UAPP_OPEN62541_VER_GE(1, 1)
/**
 * @copydoc createMonitoredItemsDataChange(Client&, const CreateMonitoredItemsRequest&,
 *          DataChangeNotificationCallback, DeleteMonitoredItemCallback)
 * @param token @completiontoken{void(CreateMonitoredItemsResponse&)}
 * @return @asyncresult{CreateMonitoredItemsResponse}
 */
template <typename CompletionToken>
auto createMonitoredItemsDataChangeAsync(
    Client& connection,
    const CreateMonitoredItemsRequest& request,
    DataChangeNotificationCallback dataChangeCallback,
    DeleteMonitoredItemCallback deleteCallback,
    CompletionToken&& token
) {
    auto contexts = detail::createMonitoredItemContexts(
        connection, request, std::move(dataChangeCallback), {}, std::move(deleteCallback)
    );
    std::vector<detail::MonitoredItemContext*> contextsPtr(contexts.size());
    std::vector<UA_Client_DataChangeNotificationCallback> dataChangeCallbacks(contexts.size());
    std::vector<UA_Client_DeleteMonitoredItemCallback> deleteCallbacks(contexts.size());
    detail::convertMonitoredItemContexts(
        contexts, contextsPtr, dataChangeCallbacks, {}, deleteCallbacks
    );
    return detail::AsyncServiceAdapter<CreateMonitoredItemsResponse>::initiate(
        connection,
        [&](UA_ClientAsyncServiceCallback callback, void* userdata) {
            throwIfBad(UA_Client_MonitoredItems_createDataChanges_async(
                opcua::detail::getHandle(connection),
                asNative(request),
                reinterpret_cast<void**>(contextsPtr.data()),  // NOLINT
                dataChangeCallbacks.data(),
                deleteCallbacks.data(),
                callback,
                userdata,
                nullptr
            ));
        },
        detail::HookToken(
            [&connection,
             subscriptionId = request.getSubscriptionId(),
             contexts = std::move(contexts)](const auto& response) mutable {
                detail::storeMonitoredItemContexts(connection, subscriptionId, response, contexts);
            },
            std::forward<CompletionToken>(token)
        )
    );
}
#endif

/**
 * Create and add a monitored item to a subscription for data change notifications.
 * Don't use this function to monitor the `EventNotifier` attribute.
 * Create a monitored item with @ref createMonitoredItemEvent instead.
 *
 * @param connection Instance of type Server or Client
 * @param subscriptionId Identifier of the subscription returned by @ref createSubscription.
 *                       Use `0U` for a local server-side monitored item.
 * @param itemToMonitor Item to monitor
 * @param monitoringMode Monitoring mode
 * @param parameters Monitoring parameters
 * @param dataChangeCallback Invoked when the monitored item is changed
 * @param deleteCallback Invoked when the monitored item is deleted
 */
template <typename T>
[[nodiscard]] MonitoredItemCreateResult createMonitoredItemDataChange(
    T& connection,
    uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    const MonitoringParametersEx& parameters,
    DataChangeNotificationCallback dataChangeCallback,
    DeleteMonitoredItemCallback deleteCallback
);

#if UAPP_OPEN62541_VER_GE(1, 1)
/**
 * @copydoc createMonitoredItemDataChange(Client&, uint32_t, const ReadValueId&, MonitoringMode,
 *          const MonitoringParametersEx&, DataChangeNotificationCallback,
 *          DeleteMonitoredItemCallback)
 * @param token @completiontoken{void(MonitoredItemCreateResult&)}
 * @return @asyncresult{MonitoredItemCreateResult}
 */
template <typename CompletionToken>
auto createMonitoredItemDataChangeAsync(
    Client& connection,
    uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    const MonitoringParametersEx& parameters,
    DataChangeNotificationCallback dataChangeCallback,
    DeleteMonitoredItemCallback deleteCallback,
    CompletionToken&& token
) {
    auto item = detail::createMonitoredItemCreateRequest(itemToMonitor, monitoringMode, parameters);
    const auto request = detail::createCreateMonitoredItemsRequest(
        subscriptionId, parameters.timestamps, {&item, 1}
    );
    return createMonitoredItemsDataChangeAsync(
        connection,
        asWrapper<CreateMonitoredItemsRequest>(request),
        std::move(dataChangeCallback),
        std::move(deleteCallback),
        detail::TransformToken(
            detail::wrapSingleResultWithStatus<
                MonitoredItemCreateResult,
                CreateMonitoredItemsResponse>,
            std::forward<CompletionToken>(token)
        )
    );
}
#endif

/**
 * Create and add monitored items to a subscription for event notifications.
 * The `attributeId` of ReadValueId must be set to AttributeId::EventNotifier.
 *
 * @param connection Instance of type Client
 * @param request Create monitored items request
 * @param eventCallback Invoked when an event is published
 * @param deleteCallback Invoked when the monitored item is deleted
 */
[[nodiscard]] CreateMonitoredItemsResponse createMonitoredItemsEvent(
    Client& connection,
    const CreateMonitoredItemsRequest& request,
    EventNotificationCallback eventCallback,
    DeleteMonitoredItemCallback deleteCallback
);

#if UAPP_OPEN62541_VER_GE(1, 1)
/**
 * @copydoc createMonitoredItemsEvent(Client&, const CreateMonitoredItemsRequest&,
 *          EventNotificationCallback, DeleteMonitoredItemCallback)
 * @param token @completiontoken{void(CreateMonitoredItemsResponse&)}
 * @return @asyncresult{CreateMonitoredItemsResponse}
 */
template <typename CompletionToken>
auto createMonitoredItemsEventAsync(
    Client& connection,
    const CreateMonitoredItemsRequest& request,
    EventNotificationCallback eventCallback,
    DeleteMonitoredItemCallback deleteCallback,
    CompletionToken&& token
) {
    auto contexts = detail::createMonitoredItemContexts(
        connection, request, {}, std::move(eventCallback), std::move(deleteCallback)
    );
    std::vector<detail::MonitoredItemContext*> contextsPtr(contexts.size());
    std::vector<UA_Client_EventNotificationCallback> eventCallbacks(contexts.size());
    std::vector<UA_Client_DeleteMonitoredItemCallback> deleteCallbacks(contexts.size());
    detail::convertMonitoredItemContexts(
        contexts, contextsPtr, {}, eventCallbacks, deleteCallbacks
    );
    return detail::AsyncServiceAdapter<CreateMonitoredItemsResponse>::initiate(
        connection,
        [&](UA_ClientAsyncServiceCallback callback, void* userdata) {
            throwIfBad(UA_Client_MonitoredItems_createEvents_async(
                opcua::detail::getHandle(connection),
                asNative(request),
                reinterpret_cast<void**>(contextsPtr.data()),  // NOLINT
                eventCallbacks.data(),
                deleteCallbacks.data(),
                callback,
                userdata,
                nullptr
            ));
        },
        detail::HookToken(
            [&connection,
             subscriptionId = request.getSubscriptionId(),
             contexts = std::move(contexts)](const auto& response) mutable {
                detail::storeMonitoredItemContexts(connection, subscriptionId, response, contexts);
            },
            std::forward<CompletionToken>(token)
        )
    );
}
#endif

/**
 * Create and add a monitored item to a subscription for event notifications.
 * The `attributeId` of ReadValueId must be set to AttributeId::EventNotifier.
 *
 * @param connection Instance of type Client
 * @param subscriptionId Identifier of the subscription returned by @ref createSubscription
 * @param itemToMonitor Item to monitor
 * @param monitoringMode Monitoring mode
 * @param parameters Monitoring parameters
 * @param eventCallback Invoked when an event is published
 * @param deleteCallback Invoked when the monitored item is deleted
 */
[[nodiscard]] MonitoredItemCreateResult createMonitoredItemEvent(
    Client& connection,
    uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    const MonitoringParametersEx& parameters,
    EventNotificationCallback eventCallback,
    DeleteMonitoredItemCallback deleteCallback = {}
);

#if UAPP_OPEN62541_VER_GE(1, 1)
/**
 * @copydoc createMonitoredItemEvent(Client&, uint32_t, const ReadValueId&, MonitoringMode,
 *          const MonitoringParametersEx&, EventNotificationCallback, DeleteMonitoredItemCallback)
 * @param token @completiontoken{void(MonitoredItemCreateResult&)}
 * @return @asyncresult{MonitoredItemCreateResult}
 */
template <typename CompletionToken>
auto createMonitoredItemEventAsync(
    Client& connection,
    uint32_t subscriptionId,
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    const MonitoringParametersEx& parameters,
    EventNotificationCallback eventCallback,
    DeleteMonitoredItemCallback deleteCallback,
    CompletionToken&& token
) {
    auto item = detail::createMonitoredItemCreateRequest(itemToMonitor, monitoringMode, parameters);
    const auto request = detail::createCreateMonitoredItemsRequest(
        subscriptionId, parameters.timestamps, {&item, 1}
    );
    return createMonitoredItemsEventAsync(
        connection,
        asWrapper<CreateMonitoredItemsRequest>(request),
        std::move(eventCallback),
        std::move(deleteCallback),
        detail::TransformToken(
            detail::wrapSingleResultWithStatus<
                MonitoredItemCreateResult,
                CreateMonitoredItemsResponse>,
            std::forward<CompletionToken>(token)
        )
    );
}
#endif

/**
 * @}
 * @defgroup ModifyMonitoredItems ModifyMonitoredItems service
 * Modify a monitored items of a subscription.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.3
 * @{
 */

/**
 * Modify monitored items of a subscription.
 *
 * @param connection Instance of type Client
 * @param request Modify monitored items request
 */
ModifyMonitoredItemsResponse modifyMonitoredItems(
    Client& connection, const ModifyMonitoredItemsRequest& request
) noexcept;

#if UAPP_OPEN62541_VER_GE(1, 1)
/**
 * @copydoc modifyMonitoredItems(Client&, const ModifyMonitoredItemsRequest&)
 * @param token @completiontoken{void(ModifyMonitoredItemsResponse&)}
 * @return @asyncresult{ModifyMonitoredItemsResponse}
 */
template <typename CompletionToken>
auto modifyMonitoredItemsAsync(
    Client& connection, const ModifyMonitoredItemsRequest& request, CompletionToken&& token
) {
    return detail::AsyncServiceAdapter<ModifyMonitoredItemsResponse>::initiate(
        connection,
        [&](UA_ClientAsyncServiceCallback callback, void* userdata) {
            throwIfBad(UA_Client_MonitoredItems_modify_async(
                opcua::detail::getHandle(connection), asNative(request), callback, userdata, nullptr
            ));
        },
        std::forward<CompletionToken>(token)
    );
}
#endif

/**
 * Modify a monitored item of a subscription.
 *
 * @param connection Instance of type Client
 * @param subscriptionId Identifier of the subscription returned by @ref createSubscription
 * @param monitoredItemId Identifier of the monitored item
 * @param parameters Monitoring parameters
 */
inline MonitoredItemModifyResult modifyMonitoredItem(
    Client& connection,
    uint32_t subscriptionId,
    uint32_t monitoredItemId,
    const MonitoringParametersEx& parameters
) noexcept {
    auto item = detail::createMonitoredItemModifyRequest(monitoredItemId, parameters);
    auto request = detail::createModifyMonitoredItemsRequest(subscriptionId, parameters, item);
    auto response = modifyMonitoredItems(
        connection, asWrapper<ModifyMonitoredItemsRequest>(request)
    );
    return detail::wrapSingleResultWithStatus<MonitoredItemModifyResult>(response);
}

#if UAPP_OPEN62541_VER_GE(1, 1)
/**
 * @copydoc modifyMonitoredItems(Client&, uint32_t, uint32_t, const MonitoringParametersEx&)
 * @param token @completiontoken{void(MonitoredItemModifyResult&)}
 * @return @asyncresult{MonitoredItemModifyResult}
 */
template <typename CompletionToken>
auto modifyMonitoredItemAsync(
    Client& connection,
    uint32_t subscriptionId,
    uint32_t monitoredItemId,
    const MonitoringParametersEx& parameters,
    CompletionToken&& token
) {
    auto item = detail::createMonitoredItemModifyRequest(monitoredItemId, parameters);
    auto request = detail::createModifyMonitoredItemsRequest(subscriptionId, parameters, item);
    return modifyMonitoredItemsAsync(
        connection,
        asWrapper<ModifyMonitoredItemsRequest>(request),
        detail::TransformToken(
            detail::wrapSingleResultWithStatus<
                MonitoredItemModifyResult,
                UA_ModifyMonitoredItemsResponse>,
            std::forward<CompletionToken>(token)
        )
    );
}
#endif

/**
 * @}
 * @defgroup SetMonitoringMode SetMonitoringMode service
 * Set the monitoring mode of a monitored items.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.4
 * @{
 */

/**
 * Set the monitoring mode of monitored items.
 *
 * @param connection Instance of type Client
 * @param request Set monitoring mode request
 */
SetMonitoringModeResponse setMonitoringMode(
    Client& connection, const SetMonitoringModeRequest& request
) noexcept;

/**
 * @copydoc setMonitoringMode(Client&, const SetMonitoringModeRequest&)
 * @param token @completiontoken{void(SetMonitoringModeResponse&)}
 * @return @asyncresult{SetMonitoringModeResponse}
 */
template <typename CompletionToken>
auto setMonitoringModeAsync(
    Client& connection, const SetMonitoringModeRequest& request, CompletionToken&& token
) {
    return detail::sendRequestAsync<SetMonitoringModeRequest, SetMonitoringModeResponse>(
        connection, request, std::forward<CompletionToken>(token)
    );
}

/**
 * Set the monitoring mode of a monitored item.
 *
 * @param connection Instance of type Client
 * @param subscriptionId Identifier of the subscription returned by @ref createSubscription
 * @param monitoredItemId Identifier of the monitored item
 * @param monitoringMode Monitoring mode
 */
inline StatusCode setMonitoringMode(
    Client& connection,
    uint32_t subscriptionId,
    uint32_t monitoredItemId,
    MonitoringMode monitoringMode
) noexcept {
    const auto request = detail::createSetMonitoringModeRequest(
        subscriptionId, {&monitoredItemId, 1}, monitoringMode
    );
    return detail::getSingleStatus(
        setMonitoringMode(connection, asWrapper<SetMonitoringModeRequest>(request))
    );
}

/**
 * @copydoc setMonitoringMode(Client&, uint32_t, uint32_t, MonitoringMode)
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 */
template <typename CompletionToken>
auto setMonitoringModeAsync(
    Client& connection,
    uint32_t subscriptionId,
    uint32_t monitoredItemId,
    MonitoringMode monitoringMode,
    CompletionToken&& token
) {
    const auto request = detail::createSetMonitoringModeRequest(
        subscriptionId, {&monitoredItemId, 1}, monitoringMode
    );
    return setMonitoringModeAsync(
        connection,
        asWrapper<SetMonitoringModeRequest>(request),
        detail::TransformToken(
            detail::getSingleStatus<SetMonitoringModeResponse>, std::forward<CompletionToken>(token)
        )
    );
}

/**
 * @}
 * @defgroup SetTriggering SetTriggering service
 * Create and delete triggering links for a triggering item.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.1.6
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.5
 * @{
 */

/**
 * Add and delete triggering links of monitored items.
 * The triggering item and the items to report shall belong to the same subscription.
 * @note Supported since open62541 v1.2
 *
 * @param connection Instance of type Client
 * @param request Set triggering request
 */
SetTriggeringResponse setTriggering(
    Client& connection, const SetTriggeringRequest& request
) noexcept;

/**
 * @copydoc setTriggering
 * @param token @completiontoken{void(SetTriggeringResponse&)}
 * @return @asyncresult{SetTriggeringResponse}
 */
template <typename CompletionToken>
auto setTriggeringAsync(
    Client& connection, const SetTriggeringRequest& request, CompletionToken&& token
) {
    return detail::sendRequestAsync<SetTriggeringRequest, SetTriggeringResponse>(
        connection, request, std::forward<CompletionToken>(token)
    );
}

/**
 * @}
 * @defgroup DeleteMonitoredItems DeleteMonitoredItems service
 * Delete a monitored items from subscriptions.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.6
 * @{
 */

/**
 * Delete monitored items from a subscription.
 *
 * @param connection Instance of type Client
 * @param request Delete monitored items request
 */
DeleteMonitoredItemsResponse deleteMonitoredItems(
    Client& connection, const DeleteMonitoredItemsRequest& request
) noexcept;

#if UAPP_OPEN62541_VER_GE(1, 1)
/**
 * @copydoc deleteMonitoredItems
 * @param token @completiontoken{void(DeleteMonitoredItemsResponse&)}
 * @return @asyncresult{DeleteMonitoredItemsResponse}
 */
template <typename CompletionToken>
auto deleteMonitoredItemsAsync(
    Client& connection, const DeleteMonitoredItemsRequest& request, CompletionToken&& token
) {
    return detail::AsyncServiceAdapter<DeleteMonitoredItemsResponse>::initiate(
        connection,
        [&](UA_ClientAsyncServiceCallback callback, void* userdata) {
            throwIfBad(UA_Client_MonitoredItems_delete_async(
                opcua::detail::getHandle(connection), asNative(request), callback, userdata, nullptr
            ));
        },
        std::forward<CompletionToken>(token)
    );
}
#endif

/**
 * Delete a monitored item from a subscription.
 *
 * @param connection Instance of type Server or Client
 * @param subscriptionId Identifier of the subscription returned by @ref createSubscription.
 *                       Use `0U` for a local server-side monitored item.
 * @param monitoredItemId Identifier of the monitored item
 */
template <typename T>
StatusCode deleteMonitoredItem(T& connection, uint32_t subscriptionId, uint32_t monitoredItemId);

#if UAPP_OPEN62541_VER_GE(1, 1)
/**
 * @copydoc deleteMonitoredItem
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 */
template <typename CompletionToken>
auto deleteMonitoredItemAsync(
    Client& connection, uint32_t subscriptionId, uint32_t monitoredItemId, CompletionToken&& token
) {
    const auto request = detail::createDeleteMonitoredItemsRequest(
        subscriptionId, {&monitoredItemId, 1}
    );
    return deleteMonitoredItemsAsync(
        connection,
        asWrapper<DeleteMonitoredItemsRequest>(request),
        detail::TransformToken(
            detail::getSingleStatus<DeleteMonitoredItemsResponse>,
            std::forward<CompletionToken>(token)
        )
    );
}
#endif

/**
 * @}
 * @}
 */

}  // namespace opcua::services

#endif
