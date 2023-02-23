#pragma once

#include "open62541pp/ApplicationDescription.h"

#include <string>

namespace opcua {

struct Endpoint {
	std::string url;
	ApplicationDescription server;
	std::string serverCertificate;
}

} // namespace opcua
