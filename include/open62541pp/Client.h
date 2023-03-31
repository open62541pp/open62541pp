// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "open62541pp/Endpoint.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Logger.h"
#include "open62541pp/NodeId.h"
#include "open62541pp/Variant.h"

#include <open62541/client.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace opcua {

// forward declaration
class NodeId;
class Node;

/// Login credentials.

class Client {

public:
    Client();

    ~Client();

    // // delete unused constructors
    Client(Client& other) = delete;
    Client(const Client& other) = delete;
    Client(Client&& other) = delete;

    /**
       @brief find servers and return a vector of server names and their urls
       @param[in] uri of network to search for servers
       @return vector of pairs of servername and urls
       @warning throws opcua::BadStatus
    */
    [[nodiscard]] std::vector<std::pair<std::string, std::string>> findServers(std::string_view url
    );

    /**
       @brief get a vector of endpoints
       @pparam[in] url of the server
       @return vector of Endpoints
       @warning throws opcua::BadStatus
    */
    [[nodiscard]] std::vector<opcua::Endpoint> getEndpoints(std::string_view url);

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

    /**
     * @brief Read Value Attribute from a node
     *
     * @param nodeId[in] id of the node
     * @return attribute value in a varian form
     */
    Variant readValueAttribute(const NodeId& nodeId);

    /**
     * @brief Write a value attribute to a node
     *
     * @param nodeId[in] id of the node
     * @param value[in] to be written
     */
    void writeValueAttribute(const NodeId& nodeId, const Variant& value);

    /**
     * @brief accessor to the internal UA_Client
     *
     * @return UA_Client*
     */
    UA_Client* handle() noexcept;

    /**
     * @brief const accessor to the internal UA_Client
     *
     * @return const UA_Client*
     */
    const UA_Client* handle() const noexcept;

private:
    struct PrivateData;
    std::unique_ptr<PrivateData> m_d;
};
}  // namespace opcua