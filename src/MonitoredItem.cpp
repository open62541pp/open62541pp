#include "open62541pp/MonitoredItem.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/Subscription.h"
#include "open62541pp/detail/ClientContext.h"
#include "open62541pp/detail/ServerContext.h"
#include "open62541pp/detail/open62541/common.h"  // UA_STATUSCODE_BADMONITOREDITEMIDINVALID

namespace opcua {

template <typename T>
Subscription<T> MonitoredItem<T>::subscription() const noexcept {
    return Subscription<T>(connection_, subscriptionId_);
}

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
const NodeId& MonitoredItem<T>::getNodeId() const {
    return getMonitoredItemContext(connection_, subscriptionId_, monitoredItemId_)
        .itemToMonitor.getNodeId();
}

template <typename T>
AttributeId MonitoredItem<T>::getAttributeId() const {
    return getMonitoredItemContext(connection_, subscriptionId_, monitoredItemId_)
        .itemToMonitor.getAttributeId();
}

// explicit template instantiations
template Subscription<Client> MonitoredItem<Client>::subscription() const noexcept;
template Subscription<Server> MonitoredItem<Server>::subscription() const noexcept;
template const NodeId& MonitoredItem<Client>::getNodeId() const;
template const NodeId& MonitoredItem<Server>::getNodeId() const;
template AttributeId MonitoredItem<Client>::getAttributeId() const;
template AttributeId MonitoredItem<Server>::getAttributeId() const;

}  // namespace opcua

#endif
