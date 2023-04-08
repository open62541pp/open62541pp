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

    /**
     * @brief Get the application URI
     * @return uri
     */
    const std::string& getUri() const;

    /**
     * @brief set the application URI
     * @param uri
     */
    void setUri(const std::string& uri);

    /**
     * @brief Get the product URI
     * @return product URI
     */
    const std::string& getProductUri() const;

    /**
     * @brief set the product URI
     * @param productUri
     */
    void setProductUri(const std::string& productUri);

    /**
     * @brief Get the application name
     * @return name
     */
    const LocalizedText& getName() const;

    /**
     * @brief set the application name
     * @param name
     */
    void setName(const LocalizedText& name);

    /**
     * @brief Get the application type
     * @return ApplicationType
     */
    ApplicationType getType() const;

    /**
     * @brief set the application type
     * @param type
     */
    void setType(ApplicationType type);

    /**
     * @brief Get the gateway server URI
     * @return gateway server URI
     */
    const std::string& getGatewayServerUri() const;

    /**
     * @brief set the gateway server URI
     * @param gatewayServerUri
     */
    void setGatewayServerUri(const std::string& gatewayServerUri);

    /**
     * @brief Get the discovery profile URI
     * @return discovery profile URI
     */
    const std::string& getDiscoveryProfileUri() const;

    /**
     * @brief set the discovery profile URI
     * @param discoveryProfileUri
     */
    void setDiscoveryProfileUri(const std::string& discoveryProfileUri);

    /**
     * @brief Get the discovery Urls
     * @return discovery Urls
     */
    const std::vector<std::string>& getDiscoveryUrls() const;

    /**
     * @brief set the discovery Urls
     * @param discoveryUrls
     */
    void setDiscoveryUrls(const std::vector<std::string>& discoveryUrls);
};

template <>
struct TypeConverter<ApplicationDescription> {
    using ValueType = ApplicationDescription;
    using NativeType = UA_ApplicationDescription;
    using ValidTypes = TypeList<Type::ApplicationDescription>;

    static void fromNative(const NativeType& src, ValueType& dst) {
        dst = ApplicationDescription(src);
    }

    static void toNative(const ValueType& src, NativeType& dst) {
        dst.applicationUri = detail::allocUaString(src.getUri());
        dst.productUri = detail::allocUaString(src.getProductUri());
        TypeConverter<std::string>::toNative(std::string(src.getName().getText()), dst.applicationName.text);
        TypeConverter<std::string>::toNative(std::string(src.getName().getLocale()), dst.applicationName.locale);
        dst.applicationType = static_cast<UA_ApplicationType>(src.getType());
        dst.gatewayServerUri = detail::allocUaString(src.getGatewayServerUri());
        dst.discoveryProfileUri = detail::allocUaString(src.getDiscoveryProfileUri());
        dst.discoveryUrls =
            detail::toNativeArrayAlloc<decltype(src.getDiscoveryUrls().begin()), Type::String>(
                src.getDiscoveryUrls().begin(), src.getDiscoveryUrls().end()
            );
    }
};

}  // namespace opcua