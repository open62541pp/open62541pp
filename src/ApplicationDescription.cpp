#include "open62541pp/ApplicationDescription.h"

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Helper.h"

#include "open62541_impl.h"

namespace opcua {

ApplicationDescription::ApplicationDescription (
    std::string uri,
    std::string productUri,
    std::string name,
    ApplicationType type,
    std::string gatewayServerUri,
    std::string discoveryProfileUri,
    std::vector<std::string> discoveryUrls
)
    : uri_(uri),
      productUri_(productUri),
      name_(name),
      type_(type),
      gatewayServerUri_(gatewayServerUri),
      discoveryProfileUri_(discoveryProfileUri),
      discoveryUrls_(discoveryUrls) {}

}  // namespace opcua