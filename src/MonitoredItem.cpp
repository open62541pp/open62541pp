#include "open62541pp/MonitoredItem.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS

#include <cassert>

#include "open62541pp/Client.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/Subscription.h"
#include "open62541pp/detail/ClientContext.h"
#include "open62541pp/detail/ServerContext.h"
#include "open62541pp/detail/open62541/common.h"

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
const NodeId& MonitoredItem<T>::getNodeId() const {
    return getMonitoredItemContext(connection_, subscriptionId_, monitoredItemId_)
        .itemToMonitor.getNodeId();
}

template <typename T>
AttributeId MonitoredItem<T>::getAttributeId() const {
    return getMonitoredItemContext(connection_, subscriptionId_, monitoredItemId_)
        .itemToMonitor.getAttributeId();
}

/* ----------------------------------- Client specializations ----------------------------------- */

template <>
void MonitoredItem<Client>::setMonitoringParameters(MonitoringParametersEx& parameters) {
    services::modifyMonitoredItem(connection_, subscriptionId_, monitoredItemId_, parameters);
}

template <>
void MonitoredItem<Client>::setMonitoringMode(MonitoringMode monitoringMode) {
    services::setMonitoringMode(connection_, subscriptionId_, monitoredItemId_, monitoringMode);
}

/* ---------------------------------------------------------------------------------------------- */

// explicit template instantiation
template class MonitoredItem<Server>;
template class MonitoredItem<Client>;

}  // namespace opcua

#endif
