#include "open62541pp/types/ApplicationDescription.h"
#include "open62541pp/Common.h"
#include "open62541pp/TypeConverter.h"
#include "open62541pp/TypeWrapper.h"

namespace opcua {

ApplicationDescription::ApplicationDescription(
    std::string applicationUri,
    std::string productUri,
    LocalizedText applicationName,
    ApplicationType applicationType,
    std::string gatewayServerUri,
    std::string discoveryProfileUri,
    std::vector<std::string> discoveryUrls
) {
        TypeConverter<std::string>::toNative(applicationUri, handle()->applicationUri);
        TypeConverter<std::string>::toNative(productUri, handle()->productUri);
        TypeConverter<LocalizedText>::toNative(applicationName, handle()->applicationName);
        handle()->applicationType = static_cast<UA_ApplicationType> (applicationType);
        TypeConverter<std::string>::toNative(gatewayServerUri, handle()->gatewayServerUri);
        TypeConverter<std::string>::toNative(discoveryProfileUri, handle()->discoveryProfileUri);

        handle()->discoveryUrlsSize = discoveryUrls.size();
        handle()->discoveryUrls = detail::toNativeArrayAlloc<decltype(discoveryUrls.begin()), Type::String>(discoveryUrls.begin(), discoveryUrls.end());


}

std::string_view const& ApplicationDescription::getUri() const noexcept {
        return asWrapper<std::string_view>(handle()->applicationUri);
}

std::string_view& ApplicationDescription::getUri() noexcept {
        return asWrapper<std::string_view>(handle()->applicationUri);
}

std::string_view const& ApplicationDescription::getProductUri() const noexcept {
        return asWrapper<std::string_view>(handle()->productUri);
}

std::string_view& ApplicationDescription::getProductUri() noexcept {
        return asWrapper<std::string_view>(handle()->productUri);
}

LocalizedText const& ApplicationDescription::getName() const noexcept {
        return asWrapper<LocalizedText>(handle()->applicationName);
}

LocalizedText& ApplicationDescription::getName() noexcept {
        return asWrapper<LocalizedText>(handle()->applicationName);
}

ApplicationType const& ApplicationDescription::getType() const noexcept {
        return asWrapper<ApplicationType>(handle()->applicationType);
}

ApplicationType& ApplicationDescription::getType() noexcept {
        return asWrapper<ApplicationType>(handle()->applicationType);
}

std::string_view const& ApplicationDescription::getGatewayServerUri() const noexcept {
        return asWrapper<std::string_view>(handle()->gatewayServerUri);
}

std::string_view& ApplicationDescription::getGatewayServerUri() noexcept {
        return asWrapper<std::string_view>(handle()->gatewayServerUri);
}

std::string_view const& ApplicationDescription::getDiscoveryProfileUri() const noexcept {
        return asWrapper<std::string_view>(handle()->discoveryProfileUri);
}

std::string_view& ApplicationDescription::getDiscoveryProfileUri() noexcept {
        return asWrapper<std::string_view>(handle()->discoveryProfileUri);
}

//std::vector<std::string> const& ApplicationDescription::getDiscoveryUrls() const noexcept {
//        return asWrapper<std::vector<std::string>>(handle()->discoveryUrls);
//}

//std::vector<std::string>& ApplicationDescription::getDiscoveryUrls() noexcept {
//        return asWrapper<std::vector<std::string>>(handle()->discoveryUrls);
//}

}  // namespace opcua