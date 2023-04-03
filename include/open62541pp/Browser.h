// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
