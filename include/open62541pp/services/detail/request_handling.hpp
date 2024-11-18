#pragma once

#include <algorithm>  // transform
#include <type_traits>  // remove_const_t
#include <vector>

#include "open62541pp/common.hpp"  // AttributeId
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/detail/string_utils.hpp"  // toNativeString
#include "open62541pp/span.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/types_composed.hpp"
#include "open62541pp/wrapper.hpp"  // asNative

namespace opcua::services::detail {

template <typename T>
auto* getPointer(T& value) noexcept {
    // NOLINTNEXTLINE(*-const-cast), request object won't be modified
    return const_cast<std::remove_const_t<T>*>(&value);
}

template <typename T>
auto* getPointer(Span<T> array) noexcept {
    // NOLINTNEXTLINE(*-const-cast), request object won't be modified
    return const_cast<std::remove_const_t<T>*>(array.data());
}

template <typename T>
auto* getNativePointer(T& wrapper) noexcept {
    return asNative(getPointer(wrapper));
}

template <typename T>
auto* getNativePointer(Span<T> array) noexcept {
    return asNative(getPointer(array));
}

template <typename T>
ExtensionObject wrapNodeAttributes(const T& attributes) noexcept {
    // NOLINTNEXTLINE(*-const-cast), won't be modified
    return ExtensionObject::fromDecoded(const_cast<T&>(attributes));
}

inline UA_AddNodesItem createAddNodesItem(
    const NodeId& parentId,
    const NodeId& referenceType,
    const NodeId& id,
    std::string_view browseName,
    NodeClass nodeClass,
    const ExtensionObject& nodeAttributes,
    const NodeId& typeDefinition
) noexcept {
    UA_AddNodesItem item{};
    item.parentNodeId.nodeId = parentId;
    item.referenceTypeId = referenceType;
    item.requestedNewNodeId.nodeId = id;
    item.browseName.namespaceIndex = id.getNamespaceIndex();
    item.browseName.name = opcua::detail::toNativeString(browseName);
    item.nodeClass = static_cast<UA_NodeClass>(nodeClass);
    item.nodeAttributes = nodeAttributes;
    item.typeDefinition.nodeId = typeDefinition;
    return item;
}

inline UA_AddNodesRequest createAddNodesRequest(UA_AddNodesItem& item) noexcept {
    UA_AddNodesRequest request{};
    request.nodesToAddSize = 1;
    request.nodesToAdd = &item;
    return request;
}

inline UA_AddReferencesItem createAddReferencesItem(
    const NodeId& sourceId, const NodeId& referenceType, bool forward, const NodeId& targetId
) noexcept {
    UA_AddReferencesItem item{};
    item.sourceNodeId = sourceId;
    item.referenceTypeId = referenceType;
    item.isForward = forward;
    item.targetNodeId.nodeId = targetId;
    return item;
}

inline UA_AddReferencesRequest createAddReferencesRequest(UA_AddReferencesItem& item) noexcept {
    UA_AddReferencesRequest request{};
    request.referencesToAddSize = 1;
    request.referencesToAdd = &item;
    return request;
}

inline UA_DeleteNodesItem createDeleteNodesItem(const NodeId& id, bool deleteReferences) noexcept {
    UA_DeleteNodesItem item{};
    item.nodeId = id;
    item.deleteTargetReferences = deleteReferences;
    return item;
}

inline UA_DeleteNodesRequest createDeleteNodesRequest(UA_DeleteNodesItem& item) noexcept {
    UA_DeleteNodesRequest request{};
    request.nodesToDeleteSize = 1;
    request.nodesToDelete = &item;
    return request;
}

inline UA_DeleteReferencesItem createDeleteReferencesItem(
    const NodeId& sourceId,
    const NodeId& referenceType,
    bool isForward,
    const NodeId& targetId,
    bool deleteBidirectional
) noexcept {
    UA_DeleteReferencesItem item{};
    item.sourceNodeId = sourceId;
    item.referenceTypeId = referenceType;
    item.isForward = isForward;
    item.targetNodeId.nodeId = targetId;
    item.deleteBidirectional = deleteBidirectional;
    return item;
}

inline UA_DeleteReferencesRequest createDeleteReferencesRequest(UA_DeleteReferencesItem& item
) noexcept {
    UA_DeleteReferencesRequest request{};
    request.referencesToDeleteSize = 1;
    request.referencesToDelete = &item;
    return request;
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

#ifdef UA_ENABLE_METHODCALLS

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

inline UA_CallRequest createCallRequest(UA_CallMethodRequest& item) noexcept {
    UA_CallRequest request{};
    request.methodsToCall = &item;
    request.methodsToCallSize = 1;
    return request;
}

#endif  // UA_ENABLE_METHODCALLS

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
) noexcept {
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

#ifdef UA_ENABLE_SUBSCRIPTIONS

template <typename SubscriptionParameters>
UA_CreateSubscriptionRequest createCreateSubscriptionRequest(
    const SubscriptionParameters& parameters, bool publishingEnabled
) noexcept {
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
UA_ModifySubscriptionRequest createModifySubscriptionRequest(
    IntegerId subscriptionId, const SubscriptionParameters& parameters
) noexcept {
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
    bool publishing, Span<const IntegerId> subscriptionIds
) noexcept {
    UA_SetPublishingModeRequest request{};
    request.publishingEnabled = publishing;
    request.subscriptionIdsSize = subscriptionIds.size();
    request.subscriptionIds = getPointer(subscriptionIds);
    return request;
}

inline UA_DeleteSubscriptionsRequest createDeleteSubscriptionsRequest(IntegerId& subscriptionId
) noexcept {
    UA_DeleteSubscriptionsRequest request{};
    request.subscriptionIdsSize = 1;
    request.subscriptionIds = &subscriptionId;
    return request;
}

template <typename MonitoringParameters>
inline void copyMonitoringParametersToNative(
    const MonitoringParameters& parameters, UA_MonitoringParameters& native
) noexcept {
    native.samplingInterval = parameters.samplingInterval;
    native.filter = parameters.filter;
    native.queueSize = parameters.queueSize;
    native.discardOldest = parameters.discardOldest;
}

template <typename MonitoringParameters>
UA_MonitoredItemCreateRequest createMonitoredItemCreateRequest(
    const ReadValueId& itemToMonitor,
    MonitoringMode monitoringMode,
    MonitoringParameters& parameters
) noexcept {
    UA_MonitoredItemCreateRequest request{};
    request.itemToMonitor = itemToMonitor;
    request.monitoringMode = static_cast<UA_MonitoringMode>(monitoringMode);
    copyMonitoringParametersToNative(parameters, request.requestedParameters);
    return request;
}

inline UA_CreateMonitoredItemsRequest createCreateMonitoredItemsRequest(
    IntegerId subscriptionId,
    TimestampsToReturn timestampsToReturn,
    Span<const UA_MonitoredItemCreateRequest> itemsToCreate
) noexcept {
    UA_CreateMonitoredItemsRequest request{};
    request.subscriptionId = subscriptionId;
    request.timestampsToReturn = static_cast<UA_TimestampsToReturn>(timestampsToReturn);
    request.itemsToCreateSize = itemsToCreate.size();
    request.itemsToCreate = getPointer(itemsToCreate);
    return request;
}

template <typename MonitoringParameters>
UA_MonitoredItemModifyRequest createMonitoredItemModifyRequest(
    IntegerId monitoredItemId, MonitoringParameters& parameters
) noexcept {
    UA_MonitoredItemModifyRequest item{};
    item.monitoredItemId = monitoredItemId;
    copyMonitoringParametersToNative(parameters, item.requestedParameters);
    return item;
}

template <typename MonitoringParameters>
UA_ModifyMonitoredItemsRequest createModifyMonitoredItemsRequest(
    IntegerId subscriptionId, MonitoringParameters& parameters, UA_MonitoredItemModifyRequest& item
) noexcept {
    UA_ModifyMonitoredItemsRequest request{};
    request.subscriptionId = subscriptionId;
    request.timestampsToReturn = static_cast<UA_TimestampsToReturn>(parameters.timestamps);
    request.itemsToModifySize = 1;
    request.itemsToModify = &item;
    return request;
}

inline UA_SetMonitoringModeRequest createSetMonitoringModeRequest(
    IntegerId subscriptionId, Span<const IntegerId> monitoredItemIds, MonitoringMode monitoringMode
) noexcept {
    UA_SetMonitoringModeRequest request{};
    request.subscriptionId = subscriptionId;
    request.monitoringMode = static_cast<UA_MonitoringMode>(monitoringMode);
    request.monitoredItemIdsSize = monitoredItemIds.size();
    request.monitoredItemIds = getPointer(monitoredItemIds);
    return request;
}

inline UA_SetTriggeringRequest createSetTriggeringRequest(
    IntegerId subscriptionId,
    IntegerId triggeringItemId,
    Span<const IntegerId> linksToAdd,
    Span<const IntegerId> linksToRemove
) noexcept {
    UA_SetTriggeringRequest request{};
    request.subscriptionId = subscriptionId;
    request.triggeringItemId = triggeringItemId;
    request.linksToAddSize = linksToAdd.size();
    request.linksToAdd = getPointer(linksToAdd);
    request.linksToRemoveSize = linksToRemove.size();
    request.linksToRemove = getPointer(linksToRemove);
    return request;
}

inline UA_DeleteMonitoredItemsRequest createDeleteMonitoredItemsRequest(
    IntegerId subscriptionId, Span<const IntegerId> monitoredItemIds
) noexcept {
    UA_DeleteMonitoredItemsRequest request{};
    request.subscriptionId = subscriptionId;
    request.monitoredItemIdsSize = monitoredItemIds.size();
    request.monitoredItemIds = getPointer(monitoredItemIds);
    return request;
}

#endif  // UA_ENABLE_SUBSCRIPTIONS

}  // namespace opcua::services::detail
