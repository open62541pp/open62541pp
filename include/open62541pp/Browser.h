//
// Created by eduml on 02/04/2023.
//

#pragma once

#include "Client.h"
#include "NodeClient.h"

#include <memory>
#include <vector>

namespace opcua {

class Browser {

public:
    explicit Browser (std::shared_ptr<Client> client_);
    ~Browser();

    /**
     * @brief browse the server and returns a vector of nodes from the client
     * @return vector of nodes
     */
    std::vector<NodeClient> browse();

private:
    struct PrivateData;
    std::unique_ptr<PrivateData> d_;
};

}  // namespace opcua
