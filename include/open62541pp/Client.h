// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Endpoint.h"
#include "ErrorHandling.h"

#include <memory>
#include <string_view>
#include <string>
#include <vector>

#include "open62541pp/Logger.h"

namespace opcua {

    // forward declaration
class NodeId;
class Node;

/// Login credentials.

class Client {

public:

	Client();

    ~Client();

	// delete unused constructors
	Client(Client &other) = delete;
	Client(Client const& other) = delete;
	Client(Client &&other) = delete;

	/**
	   @brief find servers and return a vector of server names and their urls
	   @param[in] uri of network to search for servers
	   @return vector of pairs of servername and urls
	   @warning throws opcua::BadStatus
	*/
    [[nodiscard]] std::vector<std::pair<std::string, std::string>>
	findServers(std::string_view url);

x	
	/**
	   @brief get a vector of endpoints
	   @pparam[in] url of the server
	   @return vector of Endpoints
	   @warning throws opcua::BadStatus
	*/
    [[nodiscard]] std::vector<Endpoint> getEndpoints(std::string_view url);

	/**
	   @brief Connect using a single url
	   @param[in] url of the server
	   @warning Throws opcua::BadStatus
	 */
	void connect(std::string_view url);

	/**
	   @brief Connect using username and a password
	   @param[in] url of the server
	   @param[in] username name of the user
	   @param[in] user password
	   @warning Throws opcua::BadStatus
	 */
	void connect(std::string_view url, std::string_view username, std::string_view password);

	

private:
	struct PrivateData;
	std::unique_ptr<PrivateData> m_d;
};
}
