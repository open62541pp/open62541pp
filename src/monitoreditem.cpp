#include "open62541pp/MonitoredItem.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/detail/ClientContext.h"
#include "open62541pp/detail/ServerContext.h"
#include "open62541pp/detail/open62541/common.h"  // UA_STATUSCODE_BADMONITOREDITEMIDINVALID

namespace opcua {

template <typename T>
inline static auto& getMonitoredItemContext(
    T& connection, uint32_t subscriptionId, uint32_t monitoredItemId
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
        .itemToMonitor.getNodeId();
}

template <typename T>
AttributeId MonitoredItem<T>::getAttributeId() {
    return getMonitoredItemContext(connection(), subscriptionId(), monitoredItemId())
        .itemToMonitor.getAttributeId();
}

// explicit template instantiations
template const NodeId& MonitoredItem<Client>::getNodeId();
template const NodeId& MonitoredItem<Server>::getNodeId();
template AttributeId MonitoredItem<Client>::getAttributeId();
template AttributeId MonitoredItem<Server>::getAttributeId();

}  // namespace opcua

#endif
