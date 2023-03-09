#pragma once

#include "open62541pp/TypeConverter.h"

#include <string>
#include <vector>

namespace opcua {

enum class ApplicationType {
	SERVER,
		CLIENT,
		CLIENT_AND_SERVER,
		DISCOVERY_SERVER,
		FORCE_32BITS
};

struct ApplicationDescription {
	std::string uri;
	std::string productUri;
	std::string name;
	ApplicationType type;
	std::string gatewayServerUri;
	std::string discoveryProfileUri;
	std::vector<std::string> discoveryUrls;
};

template <>
struct TypeConverter<ApplicationDescription> {
    using ValueType = ApplicationDescription;
    using NativeType = UA_ApplicationDescription;
    using ValidTypes = TypeList<Type::String, Type::ByteString, Type::XmlElement>;

    static void fromNative(const NativeType& src, ValueType& dst) {
		(void)src;
		(void)dst;
        // dst = detail::toString(src.);
    }

    static void toNative(const ValueType& src, NativeType& dst) {
        // UA_clear(&dst, detail::getUaDataType<Type::String>());
        // dst = detail::allocUaString(src);
		(void)src;
		(void)dst;
    }
};
}
