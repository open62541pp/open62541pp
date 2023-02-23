// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Endpoint.h"

#include <string_view>
#include <string>
#include <vector>

#include "open62541pp/Logger.h"

namespace opcua {

    // forward declaration
class NodeId;
class Node;

/// Login credentials.
struct Login {
    std::string username;
    std::string password;
};

class Client {

public:

    Client();

    ~Client();

    [[nodiscard]] std::vector<std::string> findServers(std::string_view url) noexcept;

    [[nodiscard]] Endpoints getEndpoints(std::string_view url);

private:
	struct PrivateData;
	std::unique_ptr<PrivateData> _d;
};
}
