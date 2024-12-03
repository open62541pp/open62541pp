#include "open62541pp/monitoreditem.hpp"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include "open62541pp/client.hpp"
#include "open62541pp/detail/client_context.hpp"
#include "open62541pp/detail/open62541/common.h"  // UA_STATUSCODE_BADMONITOREDITEMIDINVALID
#include "open62541pp/detail/server_context.hpp"
#include "open62541pp/exception.hpp"
#include "open62541pp/server.hpp"

namespace opcua {

template <typename T>
static auto& getMonitoredItemContext(
    T& connection, IntegerId subscriptionId, IntegerId monitoredItemId
) {
    const auto* context =
        detail::getContext(connection).monitoredItems.find({subscriptionId, monitoredItemId});
    if (context == nullptr) {
        throw BadStatus(UA_STATUSCODE_BADMONITOREDITEMIDINVALID);
    }
    return *context;
}

template <typename T>
const NodeId& MonitoredItem<T>::getNodeId() {
    return getMonitoredItemContext(connection(), subscriptionId(), monitoredItemId())
        .itemToMonitor.nodeId();
}

template <typename T>
AttributeId MonitoredItem<T>::getAttributeId() {
    return getMonitoredItemContext(connection(), subscriptionId(), monitoredItemId())
        .itemToMonitor.attributeId();
}

// explicit template instantiations
template const NodeId& MonitoredItem<Client>::getNodeId();
template const NodeId& MonitoredItem<Server>::getNodeId();
template AttributeId MonitoredItem<Client>::getAttributeId();
template AttributeId MonitoredItem<Server>::getAttributeId();

}  // namespace opcua

#endif
