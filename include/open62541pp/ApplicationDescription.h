#pragma once

#include "open62541pp/TypeConverter.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/Types.h"
#include "open62541pp/open62541.h"

#include <string>
#include <vector>

namespace opcua {

enum class ApplicationType { SERVER, CLIENT, CLIENT_AND_SERVER, DISCOVERY_SERVER, FORCE_32BITS };

class ApplicationDescription
    : TypeWrapper<UA_ApplicationDescription, UA_TYPES_APPLICATIONDESCRIPTION> {
public:
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    explicit ApplicationDescription(
        std::string uri,
        std::string productUri,
        std::string name,
        ApplicationType type,
        std::string gatewayServerUri,
        std::string discoveryProfileUri,
        std::vector<std::string> discoveryUrls
    );

private:
    std::string uri_;
    std::string productUri_;
    std::string name_;
    ApplicationType type_;
    std::string gatewayServerUri_;
    std::string discoveryProfileUri_;
    std::vector<std::string> discoveryUrls_;
};

}  // namespace opcua