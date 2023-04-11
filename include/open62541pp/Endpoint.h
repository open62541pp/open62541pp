#pragma once

#include "open62541pp/types/ApplicationDescription.h"

#include <string>

namespace opcua {

struct Endpoint {
	std::string url;
	ApplicationDescription server;
	std::string serverCertificate;

	Endpoint();
	~Endpoint();
};
} // namespace opcua
