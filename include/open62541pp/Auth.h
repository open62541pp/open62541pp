#pragma once

#include <string>

namespace opcua {

/// Login credentials.
struct Login {
    std::string username;
    std::string password;
};

}  // namespace opcua
