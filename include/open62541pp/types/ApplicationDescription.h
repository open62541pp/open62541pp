#pragma once

#include "open62541pp/Common.h"
#include "open62541pp/TypeConverter.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/open62541.h"
#include "open62541pp/types/Builtin.h"

#include <string>
#include <vector>

namespace opcua {

enum class ApplicationType {
    SERVER = UA_APPLICATIONTYPE_SERVER,
    CLIENT = UA_APPLICATIONTYPE_CLIENT,
    CLIENT_AND_SERVER = UA_APPLICATIONTYPE_CLIENTANDSERVER,
    DISCOVERY_SERVER = UA_APPLICATIONTYPE_DISCOVERYSERVER,
    FORCE_32BITS = __UA_APPLICATIONTYPE_FORCE32BIT
};

class ApplicationDescription
    : TypeWrapper<UA_ApplicationDescription, UA_TYPES_APPLICATIONDESCRIPTION> {
public:
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    ApplicationDescription(std::string applicationUri, std::string productUri, LocalizedText applicationName, ApplicationType applicationType,
                           std::string gatewayServerUri, std::string discoveryProfileUri, std::vector<std::string> discoveryUrls);

    /**
     * @brief Get the application URI
     * @return uri
     */
    std::string_view const& getUri() const noexcept;
    std::string_view& getUri() noexcept;
    /**
     * @brief Get the product URI
     * @return product URI
     */
    std::string_view const& getProductUri() const noexcept;
    std::string_view& getProductUri() noexcept;
    /**
     * @brief Get the application name
     * @return name
     */
    LocalizedText const& getName() const noexcept;
    LocalizedText& getName() noexcept;
    /**
     * @brief Get the application type
     * @return ApplicationType
     */
    ApplicationType const& getType() const noexcept;
    ApplicationType& getType() noexcept;
    /**
     * @brief Get the gateway server URI
     * @return gateway server URI
     */
    std::string_view const& getGatewayServerUri() const noexcept;
    std::string_view& getGatewayServerUri() noexcept;
    /**
     * @brief Get the discovery profile URI
     * @return discovery profile URI
     */
    std::string_view const& getDiscoveryProfileUri() const noexcept;
    std::string_view& getDiscoveryProfileUri() noexcept;
    /**
     * @brief Get the discovery Urls
     * @return discovery Urls
     * @todo adapt asWrapper to vectors
     */
//    std::vector<std::string> const& getDiscoveryUrls() const noexcept;
//    std::vector<std::string>& getDiscoveryUrls() noexcept;

};
}  // namespace opcua