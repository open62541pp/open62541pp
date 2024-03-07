#pragma once

#include <algorithm>  // transform
#include <type_traits>  // remove_const_t
#include <vector>

#include "open62541pp/Common.h"  // AttributeId, TimestampsToReturn, MonitoringMode
#include "open62541pp/Span.h"
#include "open62541pp/TypeWrapper.h"  // asNative
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/ExtensionObject.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

namespace opcua::services::detail {

template <typename T>
inline ExtensionObject wrapNodeAttributes(const T& attributes) {
    // NOLINTNEXTLINE, won't be modified
    return ExtensionObject::fromDecoded(const_cast<T&>(attributes));
}

template <typename T>
inline auto* getNativePointer(T& wrapper) noexcept {
    // NOLINTNEXTLINE, request object won't be modified
    return asNative(const_cast<std::remove_const_t<T>*>(&wrapper));
}

template <typename T>
inline auto* getNativePointer(Span<T> array) noexcept {
    // NOLINTNEXTLINE, request object won't be modified
    return asNative(const_cast<std::remove_const_t<T>*>(array.data()));
}

inline UA_ReadValueId createReadValueId(const NodeId& id, AttributeId attributeId) noexcept {
    UA_ReadValueId item{};
    item.nodeId = *id.handle();
    item.attributeId = static_cast<uint32_t>(attributeId);
    return item;
}

inline UA_ReadRequest createReadRequest(
    TimestampsToReturn timestamps, UA_ReadValueId& item
) noexcept {
    UA_ReadRequest request{};
    request.timestampsToReturn = static_cast<UA_TimestampsToReturn>(timestamps);
    request.nodesToReadSize = 1;
    request.nodesToRead = &item;
    return request;
}

inline UA_ReadRequest createReadRequest(
    TimestampsToReturn timestamps, Span<const ReadValueId> nodesToRead
) noexcept {
    UA_ReadRequest request{};
    request.timestampsToReturn = static_cast<UA_TimestampsToReturn>(timestamps);
    request.nodesToReadSize = nodesToRead.size();
    request.nodesToRead = getNativePointer(nodesToRead);
    return request;
}

inline UA_WriteValue createWriteValue(
    const NodeId& id, AttributeId attributeId, const DataValue& value
) noexcept {
    UA_WriteValue item{};
    item.nodeId = *id.handle();
    item.attributeId = static_cast<uint32_t>(attributeId);
    item.value = *value.handle();  // shallow copy, avoid copy of value
    item.value.hasValue = true;
    return item;
}

inline UA_WriteRequest createWriteRequest(UA_WriteValue& item) noexcept {
    UA_WriteRequest request{};
    request.nodesToWriteSize = 1;
    request.nodesToWrite = &item;
    return request;
}

inline UA_WriteRequest createWriteRequest(Span<const WriteValue> nodesToWrite) noexcept {
    UA_WriteRequest request{};
    request.nodesToWriteSize = nodesToWrite.size();
    request.nodesToWrite = getNativePointer(nodesToWrite);
    return request;
}

inline UA_CallMethodRequest createCallMethodRequest(
    const NodeId& objectId, const NodeId& methodId, Span<const Variant> inputArguments
) noexcept {
    UA_CallMethodRequest request{};
    request.objectId = objectId;
    request.methodId = methodId;
    request.inputArgumentsSize = inputArguments.size();
    request.inputArguments = getNativePointer(inputArguments);
    return request;
}

inline UA_BrowseRequest createBrowseRequest(
    const BrowseDescription& bd, uint32_t maxReferences
) noexcept {
    UA_BrowseRequest request{};
    request.requestedMaxReferencesPerNode = maxReferences;
    request.nodesToBrowseSize = 1;
    request.nodesToBrowse = getNativePointer(bd);
    return request;
}

inline UA_BrowseNextRequest createBrowseNextRequest(
    bool releaseContinuationPoint, const ByteString& continuationPoint
) noexcept {
    UA_BrowseNextRequest request{};
    request.releaseContinuationPoints = releaseContinuationPoint;
    request.continuationPointsSize = 1;
    request.continuationPoints = getNativePointer(continuationPoint);
    return request;
}

inline UA_TranslateBrowsePathsToNodeIdsRequest createTranslateBrowsePathsToNodeIdsRequest(
    const BrowsePath& browsePath
) {
    UA_TranslateBrowsePathsToNodeIdsRequest request{};
    request.browsePathsSize = 1;
    request.browsePaths = getNativePointer(browsePath);
    return request;
}

inline BrowsePath createBrowsePath(const NodeId& origin, Span<const QualifiedName> browsePath) {
    std::vector<RelativePathElement> relativePathElements(browsePath.size());
    std::transform(
        browsePath.begin(),
        browsePath.end(),
        relativePathElements.begin(),
        [](const auto& qn) {
            return RelativePathElement(ReferenceTypeId::HierarchicalReferences, false, true, qn);
        }
    );
    return {origin, RelativePath(relativePathElements)};
}

// avoid include of services/Subscription.h for definition of SubscriptionParameters

template <typename SubscriptionParameters>
inline UA_CreateSubscriptionRequest createCreateSubscriptionRequest(
    const SubscriptionParameters& parameters, bool publishingEnabled
) {
    UA_CreateSubscriptionRequest request{};
    request.requestedPublishingInterval = parameters.publishingInterval;
    request.requestedLifetimeCount = parameters.lifetimeCount;
    request.requestedMaxKeepAliveCount = parameters.maxKeepAliveCount;
    request.maxNotificationsPerPublish = parameters.maxNotificationsPerPublish;
    request.publishingEnabled = publishingEnabled;
    request.priority = parameters.priority;
    return request;
}

template <typename SubscriptionParameters>
inline UA_ModifySubscriptionRequest createModifySubscriptionRequest(
    uint32_t subscriptionId, const SubscriptionParameters& parameters
) {
    UA_ModifySubscriptionRequest request{};
    request.subscriptionId = subscriptionId;
    request.requestedPublishingInterval = parameters.publishingInterval;
    request.requestedLifetimeCount = parameters.lifetimeCount;
    request.requestedMaxKeepAliveCount = parameters.maxKeepAliveCount;
    request.maxNotificationsPerPublish = parameters.maxNotificationsPerPublish;
    request.priority = parameters.priority;
    return request;
}

inline UA_SetPublishingModeRequest createSetPublishingModeRequest(
    Span<const uint32_t> subscriptionIds, bool publishing
) {
    UA_SetPublishingModeRequest request{};
    request.publishingEnabled = publishing;
    request.subscriptionIdsSize = subscriptionIds.size();
    request.subscriptionIds = const_cast<uint32_t*>(subscriptionIds.data());  // NOLINT
    return request;
}

template <typename MonitoringParameters>
inline void copyMonitoringParametersToNative(
    const MonitoringParameters& parameters, UA_MonitoringParameters& native
) {
    native.samplingInterval = parameters.samplingInterval;
    native.filter = parameters.filter;
    native.queueSize = parameters.queueSize;
    native.discardOldest = parameters.discardOldest;
}

template <typename MonitoringParameters>
inline UA_MonitoredItemCreateRequest createMonitoredItemCreateRequest(
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    MonitoringParameters& parameters
) {
    UA_MonitoredItemCreateRequest request{};
    request.itemToMonitor = itemToMonitor;
    request.monitoringMode = static_cast<UA_MonitoringMode>(monitoringMode);
    copyMonitoringParametersToNative(parameters, request.requestedParameters);
    return request;
}

template <typename MonitoringParameters>
inline UA_MonitoredItemModifyRequest createMonitoredItemModifyRequest(
    uint32_t monitoredItemId, MonitoringParameters& parameters
) {
    UA_MonitoredItemModifyRequest item{};
    item.monitoredItemId = monitoredItemId;
    copyMonitoringParametersToNative(parameters, item.requestedParameters);
    return item;
}

template <typename MonitoringParameters>
inline UA_ModifyMonitoredItemsRequest createModifyMonitoredItemsRequest(
    uint32_t subscriptionId, MonitoringParameters& parameters, UA_MonitoredItemModifyRequest& item
) {
    UA_ModifyMonitoredItemsRequest request{};
    request.subscriptionId = subscriptionId;
    request.timestampsToReturn = static_cast<UA_TimestampsToReturn>(parameters.timestamps);
    request.itemsToModifySize = 1;
    request.itemsToModify = &item;
    return request;
}

inline UA_SetMonitoringModeRequest createSetMonitoringModeRequest(
    uint32_t subscriptionId, Span<const uint32_t> monitoredItemIds, MonitoringMode monitoringMode
) {
    UA_SetMonitoringModeRequest request{};
    request.subscriptionId = subscriptionId;
    request.monitoringMode = static_cast<UA_MonitoringMode>(monitoringMode);
    request.monitoredItemIdsSize = monitoredItemIds.size();
    request.monitoredItemIds = const_cast<uint32_t*>(monitoredItemIds.data());  // NOLINT
    return request;
}

inline UA_SetTriggeringRequest createSetTriggeringRequest(
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
    return request;
}

}  // namespace opcua::services::detail
